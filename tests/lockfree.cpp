#include <frosttools/lockfree.hpp>
#include <functional>


int main(int argc, char* argv[])
{
	{
		frosttools::threading::TaskMaster<std::function<void(void)> > manager(2);
		printf("Before start\n");
		Threading::sleep(2000);
		for(int i = 0; i < 10; i++)
		{
			printf("Adding task #%d\n", i);
			manager.schedule([i]()
			{
				printf("[Task%d] begin\n",i);
				Threading::sleep(1000);
				printf("[Task%d] is done\n",i);
			});
		}

		printf("Waiting for all tasks...\n");
		manager.wait();
		printf("All tasks are done!\n");
	}

	printf("Taskmaster destroyed!\n");
	Threading::sleep(10000);

	return 0;
}
