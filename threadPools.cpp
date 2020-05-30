#include "pch.h"


threadPools::threadPools(int threadNum) : sem(threadNum)
{
	endThread = false;
	max_thread = threadNum;
	sleepThread = 0;
}

void threadPools::start()
{
	for (int i = 0; i < max_thread; i++)
	{
		cout << "Create NO. " << i << "  thread" << endl;
		threadpools.push_back(thread([&] {
			while (!endThread)
			{
				unique_lock<mutex> lock(mtx_threadsleep);

				sleepThread++;
				// 通知当前有线程可用
				thread_available.notify_one();

				// 等待任务
				con1.wait(lock);

				sleepThread--;

				// 释放锁，避免其他线程阻塞在 con1.wait、unique_lock处导致单线程执行
				lock.unlock(); // 注意：释放锁，以下操作线程不安全

				if (endThread)
				{
					break;
				}

				// 进入信号量阻塞
				sem.acquire();

				// 确保任务队列还有任务
				mtx_acquireTask.lock();
				if (tasks.size() <= 0)
				{
					sem.release();
					mtx_acquireTask.unlock();
					continue;
				}

				function<void()> work = tasks.front();
				tasks.pop_front();
				mtx_acquireTask.unlock();

				// 线程工作
				work();

				sem.release();
			}

			cout << "Finish thread : " << this_thread::get_id() << endl;
			max_thread--;
			return;
		}));
	}

	pools_isRun = true;
}

void threadPools::doit()
{
	while (1)
	{
		if (tasks.size() <= 0)
		{
			break;
		}
		else
		{
			if (sleepThread <= 0)
			{
				unique_lock<mutex> lock(mtx_tasksmanager);
				thread_available.wait(lock, [&] { return sleepThread >= 0; });
			}

			con1.notify_one();
		}
	}

	cout << "tasks has been Done" << endl;
}

void threadPools::doit_auto()
{
	taskManager_isRun = true;

	thread_taskManager = thread([&] {
		while (!endThread && this->taskManager_isRun && max_thread > 0)
		{
			if (tasks.size() <= 0)
			{
				unique_lock<mutex> lock(mtx_tasksmanager);
				tasksmanager_wakeup.wait(lock, [&] { return tasks.size() > 0 || !this->taskManager_isRun; });
				continue;
			}
			else
			{
				if (sleepThread <= 0)
				{
					unique_lock<mutex> lock(mtx_tasksmanager);
					thread_available.wait(lock, [&] { return sleepThread > 0 || !this->taskManager_isRun; });
				}

				con1.notify_one();
			}
		}
	});

	thread_taskManager.detach();
}

// BUG
// taskManager_isRun 修改后thread_taskManager线程里的taskManager_isRun不变,导致无法结束循环（线程间通信？）
// 暂时解决：将thread_taskManager分离detach()
// 可能导致问题：资源是否会被正确释放、主线程结束后，thread_taskManager中,使用的主线程中的引用会变空
void threadPools::closeTaskManager()
{
	this->taskManager_isRun = false;

	while (thread_taskManager.joinable() != 0)
	{
		tasksmanager_wakeup.notify_one();
		thread_available.notify_one(); // 注意
		cout << thread_taskManager.joinable() << endl;
		this_thread::sleep_for(1s);
	}
}

void threadPools::addTask(function<void()> fun)
{

	mtx_acquireTask.lock(); // lock ？
	tasks.push_back(fun);
	mtx_acquireTask.unlock();

	tasksmanager_wakeup.notify_one();
}

void threadPools::join()
{
	for (auto iter = threadpools.begin(); iter != threadpools.end(); iter++)
	{
		if (iter->joinable()) // 若线程detach()则不可再join()
			iter->join();
	}
}

void threadPools::stop()
{
	endThread = true;
	taskManager_isRun = false;
	//this->closeTaskManager();
	while (max_thread > 0)
	{
		con1.notify_all();
		tasksmanager_wakeup.notify_all();
		thread_available.notify_all();
		this_thread::sleep_for(1s); // 可选
	}

	this->join(); // 子线程执行完后，需要等待子线程全部退出

	/*if (this->thread_taskManager.joinable())
	{
		this->thread_taskManager.join();
	}*/
}

threadPools::~threadPools()
{
	this->stop();
	cout << "Finish all thread" << endl;
}

// ---------------------------  测试用例  --------------------------------------
void testFun(int &sum)
{
	sum--;
}

void testThreadPoolsClass()
{
	//shared_ptr<threadPools> thread_P(new threadPools(5));
	threadPools *thread_P = new threadPools(3);
	int sum = 500;
	//threadPools tp;
	//tp.start();

	thread_P->start();
	thread_P->doit_auto();

	for (int i = 0; i < 500; i++)
	{
		thread_P->addTask([&] {
			for (int i = 0; i < 20; i++)
			{
				//cout << "Thread pid : " << this_thread::get_id() << endl;/*
				cout << "Call_fun print : " << i << endl;
				//this_thread::sleep_for(500ms);
			}

			testFun(sum);

			//this_thread::sleep_for(2s);
		});
	}

	//thread_P->doit();

	this_thread::sleep_for(1s);
	delete thread_P;

	cout << "sum　:　" << sum << endl;
	cout << "Done" << endl;
}