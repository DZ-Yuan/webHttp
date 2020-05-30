#include "MYsemaphore.h"

using namespace std;

class threadPools
{
private:
	int max_thread; // 最大线程数
	int sleepThread; // 当前等待线程数
	bool endThread; // 结束线程池
	bool taskManager_isRun;
	bool pools_isRun; // unused

	mutex mtx_threadsleep; // 线程睡眠锁，无任务时阻塞线程，搭配条件变量con1使用
	mutex mtx_rw; // 读写锁，保留（unused）
	mutex mtx_acquireTask; // 保护线程查看当前任务队列、获取任务锁
	mutex mtx_tasksmanager; // 任务分配锁，搭配条件变量thread_available使用

	condition_variable con1; // 通知、等待 线程工作
	condition_variable thread_available;  // 通知当前有线程可用
	condition_variable tasksmanager_wakeup; // 通知有新任务加入（unused）

	MYsemaphore sem;  // 信号

	thread thread_taskManager;
	vector<thread> threadpools;  // 线程池
	deque<function<void()>> tasks;  // 任务队列

public:
	threadPools() {};
	threadPools(int threadNum);

	void start();
	void addTask(function<void()> fun);
	void doit();
	void doit_auto();
	void closeTaskManager(); // BUG
	void join();
	void stop();



	~threadPools();
};