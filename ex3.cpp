#include <string>
#include <iostream>
#include <iomanip>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>

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

#define MENUSIZE sizeof(struct MenuItem)
#define ORDERSIZE sizeof(struct Order)

bool checkArguments(int, int, int, int);
void printMenu(MenuItem*, int);

int main(int args_size, char *args[])
{

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
    int shmid;
    char *shm;
    MenuItem *shm_menu;
    
    if (shmid = shmget(menu_key, MENUSIZE * 100, 0666) < 0)
    {
        perror("shmget");
		exit(1);
    }
    if ((shm = (char *)shmat(shmid, NULL, 0)) == (char *)-1)
    {
        perror("shmat");
		exit(1);
    }
    shm_menu = (MenuItem*)shm;
    for (int i=0; i<menu_items_amount; i++)
    {
        *shm_menu++ = items[i];
    }
    shm_menu = (MenuItem*)shm;
    printMenu(shm_menu, menu_items_amount);

    // Key of the orders
    key_t order_key = ftok(".", 'O');
    

    return 0;
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