#include "pch.h"
#include "MYsemaphore.h"

using namespace std;

MYsemaphore::MYsemaphore(int lockSum)
{
	this->lockSum = lockSum;
}

void MYsemaphore::acquire()
{
	unique_lock<mutex> lock(mtx);

	if (lockSum <= 0)
	{

		while (lockSum <= 0)
		{
			cont.wait(lock);
		}

		mtx_lockSum.lock();
		--lockSum;
		mtx_lockSum.unlock();

	}
	else
	{
		mtx_lockSum.lock();
		--lockSum;
		mtx_lockSum.unlock();
	}

}

void MYsemaphore::release()
{
	mtx_lockSum.lock();
	//cout << "Thread pid : " << this_thread::get_id() << "   release a lock" << endl;
	lockSum++;
	cont.notify_one();

	mtx_lockSum.unlock();
}