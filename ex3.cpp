#include <string>
#include <iostream>
#include <iomanip>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctime>
#include <stdlib.h>
#include <cstdlib>
#include <random>
#include <cmath>
#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

using namespace std;

typedef struct MenuItem
{
    int id;
    string name;
    float price;
    int total_orders;
} MenuItem;

typedef struct Order
{
    int customer_id;
    int item_id;
    int amount;
    bool done;
} Order;


#define MENUSIZE sizeof(struct MenuItem)
#define ORDERSIZE sizeof(struct Order)

bool checkArguments(int, int, int, int);
void printMenu(MenuItem*, int);
void totalOrders(MenuItem*, int);
float getRuntime(struct timeval);
float getRandom(float, float);

int main(int args_size, char *args[])
{
    // int t= system("./clean.sh");
    int status=0;
    if (args_size != 5) 
    {
        fprintf(stderr, "Input arguments are not valid\n");
        exit(1);
    }
    int simulation_time = atoi(args[1]);
    int menu_items_amount = atoi(args[2]);
    int customers_count = atoi(args[3]);
    int waiters_count = atoi(args[4]);

    if (!checkArguments(simulation_time, menu_items_amount, customers_count, waiters_count))
    {
        fprintf(stderr, "Input arguments are not valid\n");
        exit(1);
    }

    MenuItem items[] = {
        {0, "Salad", 7.5, 0},
        {1, "Pizza", 10, 0},
        {2, "Hamburger", 12, 0},
        {3, "Spaghetti", 9, 0},
        {4, "Pie", 9.5, 0},
        {5, "Milkshake", 6, 0},
        {6, "Shwarma", 11, 0},
        {7, "Sushi", 15, 0},
        {8, "Kuskus", 12, 0},
        {9, "Peanuts", 5, 0},
    };

    // Key of the menu
    key_t menu_key = ftok(".", 'M');
    int shmid_menu;
    MenuItem *shm_menu;

    shmid_menu = shmget(menu_key, MENUSIZE * menu_items_amount, IPC_CREAT|0666);
    shm_menu = (MenuItem *)shmat(shmid_menu, 0, 0);
    for (int i=0; i<menu_items_amount; i++)
    {
        shm_menu[i] = items[i];
    }


    // Key of the orders
    key_t order_key = ftok(".", 'O');
    int shmid_order;
    Order *shm_order;
    shmid_order = shmget(order_key, ORDERSIZE * customers_count, IPC_CREAT|0666);
    shm_order = (Order *)shmat(shmid_order, 0, 0);
    for (int i=0; i<customers_count; i++)
    {
        shm_order[i].customer_id = i;
        shm_order[i].done = true;
    }

    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    printf( "=====Simulation arguments=====\n" );
	printf( "Simulation time: %d\n", simulation_time );
	printf( "Menu items count: %d\n", menu_items_amount );
	printf( "Customers count: %d\n", customers_count );
	printf( "Waiters count: %d\n", waiters_count );
	printf( "==============================\n" );
	printf( "%6.3f Main process ID %d start\n", getRuntime(start_time), getpid() );
    printMenu(shm_menu, menu_items_amount);
    printf( "%6.3f Main process start creating sub-process\n", getRuntime(start_time) );

    
    pid_t childpid_cust;
    
    //create semaphore
    sem_t *semForOrders;
	semForOrders = sem_open("oSem", O_CREAT | O_EXCL, 0644, 1); 
	sem_unlink ("oSem");      


    for (int i=0; i< customers_count; i++)   
    {   
        childpid_cust = fork();              //create customers
        if (childpid_cust == 0) //if it is child
        {
            srand(getpid() + time(NULL));
            printf( "%6.3f Customer %d: created PID %d PPID %d\n", getRuntime(start_time), i, getpid(), getppid() );

            while(getRuntime(start_time)<simulation_time) //while simulation time is greater work time
            {
                sleep(getRandom(3,6));  //random sllep

                // sleep(1);
                if(shm_order[i].done == true) //if last order is done
                {
                    int itemID, amount;

                    itemID = getRandom(0,menu_items_amount);   //choose item randomally
                    amount = getRandom(1,5);             //choose amount randomally
                    printf( "%6.3f Customer ID %d: reads a menu about %s",	getRuntime(start_time), i, shm_menu[itemID].name.c_str() );

                    if(rand()%2==0) //order randomally
                    {
                        sem_wait(semForOrders);  //lock enter to critical section

                        cout << "(ordered, amount " << amount << ")" << endl;
                        shm_order[i].item_id = itemID;
                        shm_order[i].amount = amount;
                        shm_order[i].done = false;
                        // cout<<shm_order[i].done<<endl;

                        sem_post(semForOrders);  //unlock enter to critical section
                    }
                    else
                        cout<<"(doesn't want to order)"<<endl;
                }

                
            }
            printf( "%6.3f Customer ID %d: PID %d end work PPID %d\n", getRuntime(start_time), i, getpid(), getppid() );
            
            exit(0);
        }
    }
    
    pid_t childpid_waiter;

    for (int i=0; i<waiters_count; i++)
    {
        childpid_waiter = fork();             //create waiters
        if (childpid_waiter == 0)             //if it is a child
        {
            srand(getpid() + time(NULL));

			printf( "%6.3f Waiter %d: created PID %d PPID %d\n", getRuntime(start_time), i, getpid(), getppid() );

            while(getRuntime(start_time)<simulation_time) //while simulation time is greater work time
            {
                sleep(1);

                sem_wait(semForOrders);
            
                for(int j = 0; j < customers_count; j++)
                {
                    if(shm_order[j].done == false)
                    {
                        shm_menu[shm_order[j].item_id].total_orders+=shm_order[j].amount;
                        shm_order[j].done = true;
                        printf( "%6.3f Waiter ID %d: performs the order of customer ID %d (%d %s)\n", getRuntime(start_time), i, shm_order[j].customer_id, 
							shm_order[j].amount, shm_menu[shm_order[j].item_id].name.c_str() );
						break;
                    }
                }

                sem_post(semForOrders);

            }
            printf( "%6.3f Waiter ID %d: PID %d end work PPID %d\n", getRuntime(start_time), i, getpid(), getppid() );
			
	
			exit(0); 
        }
    }
    while( waitpid(-1, &status , 0) != -1 );
    printMenu(shm_menu, menu_items_amount);
    totalOrders(shm_menu, menu_items_amount);
    printf( "%6.3f Main ID %d end work\n", getRuntime(start_time), getpid() );
	printf( "%6.3f End of simulation\n", getRuntime(start_time) );

    return 0;
}


float getRandom(float low, float high)
{
    random_device r;
    default_random_engine e1(r());
    uniform_real_distribution<float> distribution(low, high);
    return distribution(e1);
}

float getRuntime(struct timeval start)
{
    struct timeval now_time;
	gettimeofday(&now_time, NULL);
	float diff = (now_time.tv_sec - start.tv_sec) * 1000.0f + (now_time.tv_usec - start.tv_usec) / 1000.0f;
	return diff / 1000.0f;

}

bool checkArguments(int sim, int menu_items, int cust, int waiters)
{
    if (sim > 30)
        return false;
    if (menu_items < 5 || menu_items > 10)
        return false;
    if (cust > 10)
        return false;
    if (waiters > 3)
        return false;
    return true;
}

void printMenu(MenuItem items[], int size)
{
    cout << "=============Menu list============="<<endl;
    cout << setw(4) << left << "Id";
    cout << setw(15) << left << "Name";
    cout << setw(7) << left << "Price";
    cout << "Orders" << endl;

    for (int i=0; i < size; i++)
    {
        cout << setw(4) << left << items[i].id; 
        cout << setw(15) << left << items[i].name;
        cout << setw(7) << setprecision(2) << fixed << left << items[i].price;
        cout << items[i].total_orders << endl;
    }
    cout << "==================================="<<endl;
}

void totalOrders(MenuItem items[], int size)
{
    float totalPrice=0;
    int  totalAmount=0;
    for(int i=0;i<size;i++)
    {
        totalAmount += items[i].total_orders;
        totalPrice += float(items[i].price * items[i].total_orders);
    }
    printf("Total orders %d, for an amount %.2f ILS \n", totalAmount, totalPrice);

}