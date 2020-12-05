#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <sstream>

using namespace std;

struct Point
{
	double x1;
	double x2;
	double d;
	double group;
};

double sigma = 3;
double eps = 0.00000001;
double G = 2;
double NU = 0.3;

vector<Point> Points;
vector<vector<vector<double> > > results;

double func(double u)
{
	return 1/(1+exp(-sigma*u));
}

double dfunc(double u)
{
	return sigma*func(u)*(1-func(u));
}

double deltafunc(Point p, double w[3])
{
	double u = w[0] + p.x1*w[1] + p.x2*w[2];
	double y = func(u);
	double dy = dfunc(y);
	double delta = (y - p.d)*dy;

	return delta;
}

void inputFile()
{
	double x1,x2,d;
	ifstream input;

	input.open("input1.txt", ios::in);
	if(input.is_open())
	{
		while(input>>d>>x1>>x2)
		{
			Point p;

			p.x1 = x1;
			p.x2 = x2;
			p.group = d;

			Points.push_back(p);
		}
	}
}

double calcE(double w[3])
{
	double ret = 0.0;

	for(int i = 0; i<Points.size(); i++)
	{
		double u = w[0] + Points[i].x1*w[1] + Points[i].x2*w[2];
		double y = func(u);

		ret += pow(y - Points[i].d, 2)/2;
	}

	return ret;
}

void Sigmoid()
{
	vector<vector<double> > result;
	double w[3];

	w[0] = 3.;//(rand()%10) / 10.;
	w[1] = (rand()%10) / 10.;
	w[2] = (rand()%10) / 10.;

	double w1[3];
	int lucky = 0;
	double dEW;
	int iter = 0;
	double Et = 0.0;
	double Et1 = 0.0;
	double nu = NU;

	vector<double> el_start;

	el_start.push_back(w[0]);
	el_start.push_back(w[1]);
	el_start.push_back(w[2]);

	result.push_back(el_start);

	do 
	{
		double dE[3] = {0, 0, 0};
		double w1[3] = {0, 0, 0};

		for(int i = 0; i<Points.size();i++)
		{
			double d = deltafunc(Points[i],w);
			dE[0] += d;
			dE[1] += d*Points[i].x1;
			dE[2] += d*Points[i].x2;
		}

		iter++;

		w1[0] = w[0] -nu * dE[0];
		w1[1] = w[1] -nu * dE[1];
		w1[2] = w[2] -nu * dE[2];

		dEW = w1[0] + w1[1] + w1[2];
		Et = calcE(w);
		Et1 = calcE(w1);

		if(Et1<Et)
		{
			w[0] = w1[0];
			w[1] = w1[1];
			w[2] = w1[2];

			lucky++;
			if(lucky>2)
			{
				lucky = 0;
				nu*=2;
			}

			vector<double> el;
			el.push_back(w[0]);
			el.push_back(w[1]);
			el.push_back(w[2]);

			result.push_back(el);
		}
		else
		{
			nu /= 2;
			lucky = 0;
		}
	} 
	while(fabs(dEW) > eps && nu > eps);
	
	cout << "Number of iterations: " << iter << endl;

	results.push_back(result);
}

void Initilization(int k)
{
	for (int j = 0; j < Points.size(); j++) 
	{
		if(Points[j].group == k)
		{
			Points[j].d = 1.0;
		}
		else
		{
			Points[j].d = 0.0;
		}

		std::cout << Points[j].group<<" "<<Points[j].d << endl;
	}
}

void Calculation()
{
	srand(time(NULL));

	for (int k = 0; k < G-1; k++) 
	{
		Initilization(k);
		Sigmoid();
	}
}

void gnuprint()
{
	FILE* gnu = popen("gnuplot -persist", "w");
	if(gnu == NULL)
	{
		cout << "can`t open pipe for gnuplot" << endl;
		return;
	}

	fprintf(gnu, "set xrange[0:25]\n");
	fprintf(gnu, "set yrange[0:20]\n");
	fprintf(gnu, "unset key\n");

	int size = 0;
	int num = 0;

	for(int i = 0; i < results.size(); i++)
	{	
		if(size < results[i].size())
		{
			size = results[i].size();
			num = i;
		}
	}

	cout << "start: " << results[0][0][0] << " " << results[0][0][1] << " " <<results[0][0][2]<<endl;

	cout << "end: " << results[0][size-1][0] << " " << results[0][size-1][1] << " " <<results[0][size-1][2]<<endl;

	for(int i = 0; i < results[num].size(); ++i)
	{
		fprintf(gnu, "set multiplot\n");
		for(int j = 0; j < Points.size(); ++j)
		{
			if(Points[j].group == 3.0)
			{
				fprintf(gnu, "plot '-' ls 1\n");
			}
			else
			{
				if(Points[j].group == 1.0)
					fprintf(gnu, "plot '-' ls 3\n");
				else
				{
					if(Points[j].group == 2.0)
						fprintf(gnu, "plot '-' ls 7\n");
					else
						fprintf(gnu, "plot '-' ls 10\n");
				}
			}
			
			fprintf(gnu, "%lf %lf\n", Points[j].x1, Points[j].x2);
			fprintf(gnu, "e\n");
		}

		for(int k = 0; k < results.size(); k++)
		{

			double b,c;

			if(results[k].size()>i)
			{
				b = -results[k][i][0]/results[k][i][2];
				c = -results[k][i][1]/results[k][i][2];
			}
			else
			{
				int n = results[k].size();
				b = -results[k][n-1][0]/results[k][n-1][2];
				c = -results[k][n-1][1]/results[k][n-1][2];
			}

			fprintf(gnu, "plot '-' using 1:2 with lines\n");
			
			for(int x = -5; x <= 25; ++x)
			{
				fprintf(gnu, "%d %lf\n", x, c * x + b);
			}
			fprintf(gnu, "e\n");
		}

		fprintf(gnu, "set nomultiplot\n");
		fflush(gnu);
		sleep(1);
	}

	pclose(gnu);
	return;
}

int main(int argc, char const *argv[]) 
{
	inputFile();
	Calculation();
	gnuprint();
	return 0;
}
