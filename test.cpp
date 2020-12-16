#include <string>
#include <iostream>
#include <vector>

using namespace std;
class A{
	public:
    void run(){
        cout<<"a"<<endl;
    }
	int * ptr;
};

class B{
	public:
    void run(){
        cout<<"b"<<endl;
    }
	string str;
};
template <typename T>
class Base
{
public:

vector<T*>data;
};

int main(){
	Base<A> *a = new Base<A>();
	a->data.push_back(new A());
    a->data.back()->run();
	Base<B> *b = new Base<B>();
    b->data.push_back(new B());
    b->data.back()->run();

}
