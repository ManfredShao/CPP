#include <iostream>
using namespace std;
class Demo
{
private:
    int id;

public:
    Demo()
    {
        this->id = -1;
        num++;
    }

    Demo(int cid)
    {
        this->id = cid;
        num++;
    }
    static int num;
    void display()
    {
        cout << "this is: " << this << ", id is:" << this->id << endl;
    }

    static void display_num()
    {
        cout << "num is: " << num << endl;
    }

};

int Demo::num = 0;

int main()
{
    Demo obj;
    Demo obj1(1);
    obj.display();
    obj1.display();
    Demo::display_num();
    return 0;
}