#include <string>
#include <iostream>
#include <iomanip>
#include <sys/sem.h>

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

bool checkArguments(int, int, int, int);
void printMenu(MenuItem*);

int main(int args_size, char *args[])
{
    if (args_size != 5) 
    {
        fprintf(stderr, "Input arguments are not valid\n");
        return -1;
    }
    int simulation_time = atoi(args[1]);
    int menu_items_amount = atoi(args[2]);
    int customers_count = atoi(args[3]);
    int waiters_count = atoi(args[4]);

    if (!checkArguments(simulation_time, menu_items_amount, customers_count, waiters_count))
    {
        fprintf(stderr, "Input arguments are not valid\n");
        return -1;
    }

    MenuItem items[] = {
        {0, "Salad", 7.5, 0},
        {1, "Pizza", 10, 0},
        {2, "Hamburger", 12, 0},
        {3, "Spaghetti", 9, 0},
        {4, "Pie", 9.5, 0},
        {5, "Milkshake", 6, 0}
    };

    printMenu(items);

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

void printMenu(MenuItem items[])
{
    cout << setw(4) << left << "Id";
    cout << setw(15) << left << "Name";
    cout << setw(7) << left << "Price";
    cout << "Orders" << endl;

    for (int i=0; i < 6; i++)
    {
        cout << setw(4) << left << items[i].id; 
        cout << setw(15) << left << items[i].name;
        cout << setw(7) << setprecision(2) << fixed << left << items[i].price;
        cout << items[i].total_orders << endl;
    }
}