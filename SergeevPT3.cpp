#include <iostream>
#include <ctime>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string>
#include <map>

using namespace std;

string inputFile = "data.txt";
string outputFile = "output.txt";
string timeStampsFile = "timestamps.txt";

ofstream fout(outputFile);

const int Num = 7;
const int hashes = 500000;
const int maxlen = 1000;
const int arr[7] = { 100, 500, 1000, 5000, 10000, 50000, 100000 };
const unsigned long long module = 2147483647; //большое простое число
const int p = 41;
vector<long long> p_pow(maxlen);

//key: companyName

//простой хэш
unsigned int easyHash(string key) {
    unsigned int result = 1;

    for (int i = 0; i < key.length(); ++i)
        result = (result * (key[i] - 'a' + 1)) % hashes;

    return result % hashes;
}

//сложный хэш
unsigned int complicatedHash(string key) {
    unsigned long long hash = 0;
    for (size_t j = 0; j < key.length(); ++j)
        hash = (hash + (key[j] - 'a' + 1) * p_pow[j]) % module;

    return hash % hashes;
}

class Flight {
    int number;
    string companyName;
    unsigned long long date;
    unsigned long long time;
    int passengerNumber;
    unsigned int hash;

public:
    Flight()
    {
        this->number = 0;
        this->companyName = "companyName";
        this->date = 0;
        this->time = 0;
        this->passengerNumber = 0;
        this->hash = easyHash(this->companyName);
    }

    Flight(int number,
        string companyName,
        unsigned long long date,
        unsigned long long time,
        int passengerNumber,
        unsigned int (*hashFunc)(string))
    {
        this->number = number;
        this->companyName = companyName;
        this->date = date;
        this->time = time;
        this->passengerNumber = passengerNumber;
        this->hash = hashFunc(companyName);
    }

    friend bool operator== (const Flight& a, const Flight& b) {
        if (a.time == b.time && a.companyName == b.companyName && a.date == b.date && a.passengerNumber == b.passengerNumber)
            return true;
        return false;
    }

    friend bool operator< (const Flight& a, const Flight& b) {
        if (a.date < b.date || ((a.date == b.date) && (a.time < b.time)) ||
            ((a.date == b.date) && (a.time == b.time) && (a.companyName < b.companyName)) ||
            ((a.date == b.date) && (a.time == b.time) && (a.companyName == b.companyName) && (a.passengerNumber == b.passengerNumber)))
            return true;
        return false;
    }

    friend bool operator<= (const Flight& a, const Flight& b) {
        if (a < b || a == b)
            return true;
        return false;
    }

    friend bool operator> (const Flight& a, const Flight& b) {
        if (!(a < b) && !(a == b))
            return true;
        return false;
    }

    friend bool operator>= (const Flight& a, const Flight& b) {
        if (!(a < b))
            return true;
        return false;
    }

    friend ostream& operator<<(ostream& os, const Flight& a) {
        os << a.date << " " << a.time << " " << a.companyName << " " << a.passengerNumber << "\n";
        return os;
    }

    string getCompanyName() const {
        return this->companyName;
    }

    unsigned int getHash() const {
        return this->hash;
    }
};

vector<vector<Flight>> readTxtFile(unsigned int (*hashFunc)(string)) {
    ifstream fin(inputFile);

    vector<vector<Flight>> result;

    int dim;
    int number;
    string companyName;
    unsigned long long date;
    unsigned long long time;
    int passengerNumber;

    for (int i = 0; i < Num; ++i) {
        //Ввод числа объектов
        fin >> dim;
        vector<Flight> v;
        for (int j = 0; j < dim; ++j) {
            //Ввод полей объекта
            fin >> number >> companyName >> date >> time >> passengerNumber;
            Flight temp(number, companyName, date, time, passengerNumber, hashFunc);
            v.push_back(temp);
        }
        result.push_back(v);
    }

    return result;
}

void writeTime(string title, std::chrono::steady_clock::time_point start, std::chrono::steady_clock::time_point end, int divideBy) {
    fout << title;

    fout << chrono::duration_cast<chrono::microseconds>(end - start).count() / divideBy << " [микросекунд]\n";
}

struct HashItem {
    vector<string> values;
};

class HashTable {
    vector<HashItem> table;

public:
    HashTable() {
        table.resize(hashes);
    }

    void insert(Flight& item) {
        //fout << "insert " << item.getHash() << '\n';
        HashItem& currentItem = this->table[item.getHash()];

        //при совпадении элементов выходим
        for (auto i : currentItem.values)
            if (i == item.getCompanyName())
                return;

        currentItem.values.push_back(item.getCompanyName());
    }

    vector<string> find(Flight& item) {
        HashItem& currentItem = this->table[item.getHash()];

        return currentItem.values;
    }

    void clear() {
        this->table.clear();
        this->table.resize(hashes);
    }


    unsigned int getCollisions() {
        unsigned int result = 0;

        for (auto n : this->table)
            if (n.values.size() > 1)
                result += n.values.size() - 1;

        return result;
    }
};

int main()
{
    setlocale(LC_ALL, "Russian");

    //заполнение массива степеней
    p_pow[0] = 1;
    for (size_t i = 1; i < p_pow.size(); ++i)
        p_pow[i] = p_pow[i - 1] * p;

    vector<vector<Flight>> naiveArray = readTxtFile(easyHash);

    HashTable naiveTable;

    std::chrono::steady_clock::time_point start, end;

    fout << "Простая хэш функция:\n" << '\n';

    for (int i = 0; i < Num; ++i) {
        //заполнение таблицы
        for (int j = 0; j < naiveArray[i].size(); ++j)
            naiveTable.insert(naiveArray[i][j]);

        start = std::chrono::steady_clock::now();

        //поиск всех элементов для получения среднего результата
        for (int j = 0; j < naiveArray[i].size(); ++j) {
            Flight objectToFind = naiveArray[i][j];
            vector<string> foundItems = naiveTable.find(objectToFind);
        }

        end = std::chrono::steady_clock::now();
        fout << "\"Простая\" реализация таблицы с " + to_string(naiveArray[i].size()) + " элементами:\n";
        writeTime("На поиск в среднем уходит: ", start, end, naiveArray[i].size());
        fout << "Коллизий: " << naiveTable.getCollisions() << "\n\n";
        naiveTable.clear();
        start = end;
    }

    naiveArray.clear();

    vector<vector<Flight>> complicatedArray = readTxtFile(complicatedHash);

    HashTable complicatedTable;

    fout << "Сложная хэш функция:\n" << '\n';

    for (int i = 0; i < Num; ++i) {
        //заполнение таблицы
        for (int j = 0; j < complicatedArray[i].size(); ++j)
            complicatedTable.insert(complicatedArray[i][j]);

        start = std::chrono::steady_clock::now();

        //поиск всех элементов для получения среднего результата
        for (int j = 0; j < complicatedArray[i].size(); ++j) {
            Flight objectToFind = complicatedArray[i][j];
            vector<string> foundItems = complicatedTable.find(objectToFind);
        }

        end = std::chrono::steady_clock::now();
        fout << "\"Сложная\" реализация таблицы с " + to_string(complicatedArray[i].size()) + " элементами:\n";
        writeTime("На поиск в среднем уходит: ", start, end, complicatedArray[i].size());
        fout << "Коллизий: " << complicatedTable.getCollisions() << "\n\n";
        complicatedTable.clear();
    }

    
    fout.close();

    return 0;
}
