#include <iostream>
#include <algorithm>
using namespace std;

template <class Key, class Value>
class DictionaryBase
{
protected:
    Key *keys;
    Value *values;
    int size;
    int max_size;

public:
    DictionaryBase(int initial_size) : size(0)
    {
        max_size = 1;
        while (initial_size >= max_size)
            max_size *= 2;
        keys = new Key[max_size];
        values = new Value[max_size];
    }
    void add(Key key, Value value)
    {
        Key *tmpKey;
        Value *tmpVal;
        if (size + 1 >= max_size)
        {
            max_size *= 2;
            tmpKey = new Key[max_size];
            tmpVal = new Value[max_size];
            for (int i = 0; i < size; i++)
            {
                tmpKey[i] = keys[i];
                tmpVal[i] = values[i];
            }
            tmpKey[size] = key;
            tmpVal[size] = value;
            delete[] keys;
            delete[] values;
            keys = tmpKey;
            values = tmpVal;
        }
        else
        {
            keys[size] = key;
            values[size] = value;
        }
        size++;
    }
    void print()
    {
        for (int i = 0; i < size; i++)
            cout << "{" << keys[i] << ", " << values[i] << "}" << endl;
    }
    ~DictionaryBase()
    {
        delete[] keys;
        delete[] values;
    }
};

template <class Key, class Value>
class Dictionary : public DictionaryBase<Key, Value>
{
public:
    Dictionary(int initial_size) : DictionaryBase<Key, Value>(initial_size) {}
};

template <typename Value>
class Dictionary<int, Value> : public DictionaryBase<int, Value>
{
public:
    Dictionary(int initial_size) : DictionaryBase<int, Value>(initial_size) {}
    
    void sort() {
        for(int i = 0; i < this->size - 1; i++) {
            for(int j = 0; j < this->size - i - 1; j++) {
                if(this->keys[j] > this->keys[j + 1]) {
                    swap(this->keys[j], this->keys[j + 1]);
                    swap(this->values[j], this->values[j + 1]);
                }
            }
        }
    }
};

int main()
{
    Dictionary<const char *, const char *> dict(10);
    dict.print();
    dict.add("apple", "fruit");
    dict.add("banana", "fruit");
    dict.add("dog", "animal");
    dict.print();
    Dictionary<int, const char *> dict_specialized(10);
    dict_specialized.print();
    dict_specialized.add(100, "apple");
    dict_specialized.add(101, "banana");
    dict_specialized.add(103, "dog");
    dict_specialized.add(89, "cat");
    dict_specialized.print();
    dict_specialized.sort();
    cout << endl
         << "Sorted list:" << endl;
    dict_specialized.print();
    return 0;
}