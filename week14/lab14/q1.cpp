#include <iostream>
#include <exception>

using namespace std;

class OutOfRangeException : public exception
{
public:
    const char* what() const noexcept override
    {
        return " which out of range(0-100).";
    }
};

void calculate_average(int* a)
{
    int i = 0;
    double average = 0;
    try {
        for(; i < 4; ++i) {
            if (a[i] < 0 || a[i] > 100)
                throw OutOfRangeException();
            else
                average += a[i];
        }
        average /= 4;
        cout << "The average is: " << average << endl;
    }
    catch (const OutOfRangeException& e) {
        cerr << "The parameter " << i+1 << " is " << a[i] << e.what() << endl;
    }
}

int main() {
    cout << "Please enter marks for 4 courses:";
    int marks[4];
    for (int i = 0; i < 4; ++i) {
        cin >> marks[i];
    }
    calculate_average(marks);
    do{
        cout << "Would you want to enter another marks for 4 courses? (y/n): ";
        char choice;
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            cout << "Please enter marks for 4 courses:";
            for (int i = 0; i < 4; ++i) {
                cin >> marks[i];
            }
            calculate_average(marks);
        } else{
            cout << "Bye!" << endl;
            break;
        }
    } while (true);
    return 0;
}