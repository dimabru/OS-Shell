#include <string>
#include <iostream>
#include <iomanip>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h>
#include <ctime>
#include <stdlib.h>
#include <cstdlib>
#include <random>
#include <cmath>
#include <stdio.h>

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
    bool done = true;
} Order;

typedef struct Utils
{
    double start_time;
    int total_time;
    int menu_items_amount;
} Utils;

#define MENUSIZE sizeof(struct MenuItem)
#define ORDERSIZE sizeof(struct Order)

bool checkArguments(int, int, int, int);
void printMenu(MenuItem*, int);
double getRuntime();
float getRandom(float, float);

int main(int args_size, char *args[])
{
    system("./clean.sh");

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
    printMenu(shm_menu, menu_items_amount);

    // Key of Utils
    key_t utils_key = ftok(".", 'U');
    int shmid_utils;
    Utils *shm_utils;
    struct timespec begin;

    shmid_utils = shmget(utils_key, sizeof(struct Utils), IPC_CREAT|0666);
    shm_utils = (Utils *)shmat(shmid_utils, 0, 0);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    (*shm_utils).start_time = begin.tv_sec;
    (*shm_utils).total_time = simulation_time;

    // Key of the orders
    key_t order_key = ftok(".", 'O');
    int shmid_order;
    Order *shm_order;

    shmid_order = shmget(menu_key, ORDERSIZE * 100, IPC_CREAT|0666);
    shm_order = (Order *)shmat(shmid_order, 0, 0);
    pid_t childpid_cust;
    for (int i=0; i< 20; i++)
    {   
        childpid_cust = fork();
        if (childpid_cust == 0)
        {
            sleep(getRandom(3,6));
            cout << getRuntime() << " --- I am a child. PPID: " << getppid() << " PID: " << getpid() << endl;
            return 0;
        }
    }
    
    return 0;
}

float getRandom(float low, float high)
{
    random_device r;
    default_random_engine e1(r());
    uniform_real_distribution<float> distribution(low, high);
    return distribution(e1);
}

double getRuntime()
{
    key_t utils_key = ftok(".", 'U');
    int shmid_utils;
    Utils *shm_utils;
    struct timespec current;

    shmid_utils = shmget(utils_key, sizeof(struct Utils), IPC_CREAT|0666);
    shm_utils = (Utils *)shmat(shmid_utils, 0, 0);
    clock_gettime(CLOCK_MONOTONIC, &current);
    return current.tv_sec - (*shm_utils).start_time;
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
}