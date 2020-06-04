#pragma once
#include <mutex>
#include <condition_variable>

using namespace std;

class MYsemaphore
{
private:
	int lockSum;

	mutex mtx, mtx_lockSum;
	condition_variable cont;



public:
	MYsemaphore() {};
	MYsemaphore(int lockSum);

	void acquire();
	void release();

};
