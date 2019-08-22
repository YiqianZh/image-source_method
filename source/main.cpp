#include <iostream>
#include <vector>
#include <fstream>
#define print_point false
#define write_file false
using namespace std;

struct Plane
{
	double A;
	double B;
	double C;
	double D;
};

struct Point
{
	double x;
	double y;
	double z;
	void print()
	{
	    #if print_point
	    std::cout << "\n ( " << x << " , " << y << " , " << z << " )\n";
	    #endif
	}
};

class Wall
{
public:
	Plane pl;
	Point pt1;
	Point pt2;
	double absorbtion;
};

Point mirror(Point p, Wall w)
{
    double t = -(w.pl.A*p.x + w.pl.B*p.y + w.pl.C*p.z + w.pl.D)/(w.pl.A*w.pl.A + w.pl.B*w.pl.B + w.pl.C*w.pl.C);
    double xo = w.pl.A*t+p.x;
    double yo = w.pl.B*t+p.y;
    double zo = w.pl.C*t+p.z;
    return {2*xo-p.x, 2*yo-p.y, 2*zo-p.z};
}

int main()
{

#if write_file
    ofstream myfile;
    myfile.open ("example.txt");
#endif // write_file

    double a = 3;
    double b = 5;
    double h = 2.3;
    const int level = 6;
    const int N = 6;
    int liczba = 1;
    for(int i = 0 ; i < level ; i++)
            liczba=liczba*N;
#if write_file
    myfile<<"Wielokat(("<<-a<<","<<-b<<","<<-h<<"),("<<-a<<","<<-b<<","<<h<<"),("<<a<<","<<-b<<","<<h<<"),("<<a<<","<<-b<<","<<-h<<"))\n";
    myfile<<"Wielokat(("<<-a<<","<<b<<","<<-h<<"),("<<-a<<","<<b<<","<<h<<"),("<<a<<","<<b<<","<<h<<"),("<<a<<","<<b<<","<<-h<<"))\n";
    myfile<<"Wielokat(("<<-a<<","<<-b<<","<<-h<<"),("<<-a<<","<<-b<<","<<h<<"),("<<-a<<","<<b<<","<<h<<"),("<<-a<<","<<b<<","<<-h<<"))\n";
    myfile<<"Wielokat(("<<a<<","<<-b<<","<<-h<<"),("<<a<<","<<-b<<","<<h<<"),("<<a<<","<<b<<","<<h<<"),("<<a<<","<<b<<","<<-h<<"))\n";
    myfile<<"Wielokat(("<<a<<","<<b<<","<<h<<"),("<<-a<<","<<b<<","<<h<<"),("<<-a<<","<<-b<<","<<h<<"),("<<a<<","<<-b<<","<<h<<"))\n";
    myfile<<"Wielokat(("<<a<<","<<b<<","<<-h<<"),("<<-a<<","<<b<<","<<-h<<"),("<<-a<<","<<-b<<","<<-h<<"),("<<a<<","<<-b<<","<<-h<<"))\n";
#endif // write_file

	Wall walls[N];
	walls[0] = { {0, 0, 1, h} , {-a, -b, -h}, {a, b, -h}, 0.71 };
	walls[1] = { {0, 0, -1, h}, {-a, -b, h}, {a, b, h}, 0.68 };
	walls[2] = { {0, 1, 0, b} , {-a, -b, -h}, {a, -b, h}, 0.75 };
	walls[3] = { {0, -1, 0, b}, {-a, b, -h}, {a, b, h}, 0.72 };
	walls[4] = { {1, 0, 0, a} , {-a, -b, -h}, {-a, b, h}, 0.54 };
	walls[5] = { {-1, 0, 0, a}, {a, -b,-h}, {a, b, h}, 0.61 };

	Point source = { 0.2, 0.4, 0.3 };
	Point receiver = { 0.6, -1.7, 0.4};

#if write_file
	myfile << "S = (" << source.x << ", " << source.y << ", " << source.z << ")\n";
	myfile << "R = (" << receiver.x << ", " << receiver.y << ", " << receiver.z << ")\n";
#endif // write_file

    int tab[level];

    int xx = 8;
    for(int j = 1 ; j < liczba ; j++)
    {
    int L = j;
    bool czy_zly = false;
    bool czy_wektor = false;
    for(int i = 0 ; i<level; i++)
    {
        tab[i] = L%N;
        L=L/N;
        //std::cout << tab[i] << " - ";
    }

    //tab[0] = 1;
    //tab[1] = 2;
    //tab[2] = 0;
    Point im_s = source;
    for(int i = 0 ; i<level; i++)
    {
        im_s = mirror(im_s,walls[tab[i]]);
#if write_file
        myfile << "S" << i+1 << " = (" << im_s.x << ", " << im_s.y << ", " << im_s.z << ")\n";
#endif // write_file

    }
    im_s.print();

    int i = level-1;
    Point A = im_s;
    Point B = receiver;
    Point vN = {B.x-A.x , B.y-A.y , B.z-A.z};

    for( int i = level-1 ; i>=0 ; i--)
    {
        //std::cout << "Iter: " << i << std::endl;
        A.print();
        double t = -(walls[tab[i]].pl.A*A.x + walls[tab[i]].pl.B*A.y + walls[tab[i]].pl.C*A.z + walls[tab[i]].pl.D)/(walls[tab[i]].pl.A*vN.x + walls[tab[i]].pl.B*vN.y + walls[tab[i]].pl.C*vN.z);
        Point cross = { vN.x*t + A.x ,  vN.y*t + A.y , vN.z*t + A.z };
#if write_file
        myfile << "C" << i << " = (" << cross.x << ", " << cross.y << ", " << cross.z <<")\n";
#endif // write_file
        cross.print(); //sprawdzic czy croos jest w plane

        if( vN.x*walls[tab[i]].pl.A + vN.y*walls[tab[i]].pl.B + vN.z*walls[tab[i]].pl.C < 0)
            czy_wektor = true;
        double eps = 0.00001;
        if(cross.x - walls[tab[i]].pt1.x < -eps || cross.x - walls[tab[i]].pt2.x > eps || cross.y - walls[tab[i]].pt1.y < -eps || cross.y - walls[tab[i]].pt2.y > eps || cross.z - walls[tab[i]].pt1.z < -eps || cross.z - walls[tab[i]].pt2.z > eps)
        {
            czy_zly = true;
            break;
        }

        double t2 = -(walls[tab[i]].pl.A*vN.x + walls[tab[i]].pl.B*vN.y + walls[tab[i]].pl.C*vN.z)/(walls[tab[i]].pl.A*walls[tab[i]].pl.A + walls[tab[i]].pl.B*walls[tab[i]].pl.B + walls[tab[i]].pl.C*walls[tab[i]].pl.C);

        Point vN2 = { 2*walls[tab[i]].pl.A*t2 + vN.x , 2*walls[tab[i]].pl.B*t2 + vN.y , 2*walls[tab[i]].pl.C*t2 + vN.z };
        vN.print();
        vN2.print(); // nowy promien
        vN = vN2;
        A = cross;
        cross.print();
    }
    if(czy_zly == false && czy_wektor == false)
    {
         std::cout << j << ": ";
        for(int x = 0; x<level-1 ; x++)
            std::cout << tab[x] << " - ";
        std::cout << tab[level-1] << "\n";
    }
    }

    #if write_file
    myfile << "Odcinek(S, C0)\n";
    for(int i = 0; i<level-1; i++)
    {
        myfile << "Odcinek(C" << i <<", C" << i+1 <<")\n";
    }
    myfile << "Odcinek(C" << level-1 << ", R)\n";
    myfile.close();
    #endif // write_file

    return 0;
}
