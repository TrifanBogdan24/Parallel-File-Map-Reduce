#include <iostream>
#include <vector>


using namespace std;


int main()
{
    vector<int> v;
    v.resize(10);

    for (int i = 0; i < 10; i++) {
        v[i] = 0;
    }

    v.insert(v.begin() + 3, 897);

    for (int i = 0; i < 10; i++) {
        cout << v[i] << " ";
    }

    cout << "\n";
    return 0;
}