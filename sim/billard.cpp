//====================================================================
// Simulation dynamischer Systeme mit PLAN
//====================================================================
// Projektbeschreibung:
//
//====================================================================
#include <vcl.h>
#pragma hdrstop
#include "Plan.h"
#include "Data.h"
#include <iostream>
#define log(x) std::cout << x << std::endl;
#define N 16

static double Stocklang = 1100.0;
static double Stockradius = 30.0;
static double Stockanfangsort = -800.0;

static double my = 0.01; 	// Rollreibung
static double bande = 0.8; 	// Dämpfung durch die Bande
static double r = 30.0;  	// Kugelradius
static double ForceScale = 0.06;// Skalierungskonstante für die Kraft bei Stoss mit dem Queue
int counter = 0;

real dist(real x1, real y1, real x2, real y2) {
	return sqrt(pow(x1 - x2,2) + pow(y1 - y2, 2));
}


int dist(int x1, int y1, int x2, int y2) {
	return sqrt(pow(x1 - x2,2) + pow(y1 - y2, 2));
}

double norm(TVektor a) {
	return sqrt(a[0]*a[0] + a[1]*a[1]);
}

double dot(TVektor a, TVektor b) {
	real res = 0.0;
	for(int i = 0; i < a.Zeilen(); i++) {
		res += a[i] * b[i];
	}
	return res;
}

void vlog(TVektor v) {
	double a = v[0];
	double b = v[1];
	printf("x: %8.31e, y: %8.31e\n", a, b);
}


class Kugel {
	private:

	public:
                int n; 		// Nummer der Kugel 	
		real r; 	// Radius
		TVektor pos;	// Position pos[0] = x, pos[1] = y
		TVektor next;	// Posistion in der Nächsten Iteration
		TVektor v;	// Geschwindigkeit
		double m;	// Masse
		bool inGame;	// Ist Kugel im Spiel 	
		TColor color;

		void init_Kugel(int n, real x, real y, TColor color) {
			this->n = n;
			this->r = r;
			this->pos = TVektor(x,y);
			this->next = TVektor(x,y);
			this->v = TVektor(0.0,0.0);
			this->m = 50;
			this->inGame = true;
			this->color = color;
		}

		void move() {
			if (norm(this->next) < 0.001 ) {
				this->pos = this->next;
				this->next = TVektor(0.0,0.0);
			}
			this->pos += this->v;
			this->v *= 1 - my;
		}
};

class Billard : public TPlan {
	private:
    		double y,z,	// Koordinatesystem
		       xZ,yZ,	// Queue Ziel
		       force,	// Kraft beim Stoss mit dem Queue
                       t1,t2,
                       K0w,K0h;
                int    Mode,
                       K0On,K0Move;

		TVektor Z,K0;
		bool moving;	// Bewegen sich Kugeln?

                Kugel kugeln[N];
		//double kugeln_X[N];
		//double kugeln_Y[N];
		//TColor colors[N];

	public:
	void Init();
	void Run();
	void Reset();
        void RunTaste0();
        void RunTaste1();

	void Stoss();
	bool CheckMoving();
	void HandleHole();
	void HandleBoxCollision();
	void HandleBallCollision();
	void CheckHoles();
	void BallCCD(int);
	void DrawKugeln();
	void DrawTable();
	void Debug() {
	}

	void RunTaste10() {
		DrawKugeln();
	}

	void BildMouseDown(int x, int y){
		if ( ! moving ) Stoss();

                if ( dist(kugeln[0].pos[0], kugeln[0].pos[1], IntToX(x), IntToY(y))<r){
                        K0On = 1;
                        K0w=IntToX(x)-kugeln[0].pos[0];
                        K0h=IntToY(y)-kugeln[0].pos[1];}
                else  K0On = 0;
	}

	void BildMouseMove(int x, int y, int left){
		Z[0] = IntToX(x);
		Z[1] = IntToY(y);
	    	//xZ = IntToX(x);
	    	//yZ = IntToY(y);
                
                if(left && K0On && K0Move){
                        kugeln[0].pos[0]=IntToX(x)-K0w;
                        kugeln[0].pos[1]=IntToY(y)-K0h;
                        kugeln[0].v=0.0;
                        if(kugeln[0].pos[0]-r<-1230.0) kugeln[0].pos[0]=-1230.0+r;
                        if(kugeln[0].pos[0]>-635.0) kugeln[0].pos[0]=-635.0;
                        if(kugeln[0].pos[1]-r<-600.0) kugeln[0].pos[1]=-600.0+r;
                        if(kugeln[0].pos[1]+r>600.0) kugeln[0].pos[1]=600.0-r;}
	}

        void BildMouseUp(int x, int y){
            if ( dist(kugeln[0].pos[0], kugeln[0].pos[1], IntToX(x), IntToY(y))<r) K0Move=0;}
	
	void InitQueue(){
	    SetPen(Hellrot,5);
	    SetBrush(Hellrot);
	}

	void DrawForce(double distance){
		if(Mode==0) force = (distance*0.9 < 800) ?  distance*0.9 : 800;
                // if(Mode==1) force = ((800-(t2-t1))> 10)  ?  (800-(t2-t1)): 10;
                if(Mode==1){
                   if((t2-t1) <= 790)   force = 800-(t2-t1);
                   if((t2-t1) >= 790 && (t2-t1)<=790*2)  force = -780+(t2-t1);
                   if((t2-t1)>=790*2 &&(t2-t1) <=790*3)  force = 2380-(t2-t1);
                   if((t2-t1)>=790*3 && (t2-t1)<=790*4)  force = -2360+(t2-t1);
                   if((t2-t1)>=790*4) force = 10;
                   }
		SetPen(Rot);
		SetBrush(Rot);
		Circle(-400.0,-815.0,30.0);
		Rectangle(-400.0,-845.0,force,60.0);
		Circle(400.0,-815.0,30.0);
	}

	void DrawQueue(int show=1){
	    SetPen(Hellrot,5);
	    SetBrush(Hellrot);
	    //Circle(xK,yK,Stockradius);
	
	    SetPen(Schwarz,3);
	    MoveTo(kugeln[0].pos[0],kugeln[0].pos[1]);
	    LineTo(Z[0],Z[1]);
	    SetBrush(Klar);
	    Circle(Z[0],Z[1],Stockradius);
	    double distance = dist(kugeln[0].pos[0], kugeln[0].pos[1], Z[0], Z[1]);
	    if(distance==0) distance =  Stockradius;
	    DrawForce(distance);
	    double phi = acos((kugeln[0].pos[0]-Z[0])/distance);
	    if(Z[1]>kugeln[0].pos[1]) phi = -phi;
	    SetPen(Blau,15);
	    MoveTo(kugeln[0].pos[0]+60.0*cos(phi),kugeln[0].pos[1]+60.0*sin(phi));
	    LineTo(kugeln[0].pos[0]+Stocklang*cos(phi),kugeln[0].pos[1]+Stocklang*sin(phi));}
};


void Billard::Init() {
	ProgrammName = "Billard";

	CallRunTime = 10;
	moving = false;

	Scale(-1700.0,1700.0,0.0);
	y = IntToY(0);
	Scale(-1700.0,1700.0,-y/2.0);

	Z = TVektor(0.0,0.0);
	Reset();
}


void Billard::Run() {
        if(kugeln[0].pos[0]>-635.0)      K0Move = 0;
        if(kugeln[0].pos[0]==K0[0])    K0Move = 1;

	moving = CheckMoving();
	if ( moving ) {
                t1=t2;
		HandleBoxCollision();
		HandleBallCollision();
                CheckHoles();
		//if(counter%50 == 0) log(norm(kugeln[0].v));
		for(int i = 0; i < N; i++) {
			kugeln[i].move();
		}
	}
	if(counter%2 == 0) {
		DrawTable();	
 		DrawKugeln();	
		if ( ! moving&&!K0Move) {DrawQueue();}
	}
	CallRun = True;
	counter++;
        t2=counter;
        //printf("t1: %8.31e, t2: %8.31e, t3: %8.31e\n", t1, t2);
        InsertTaste(0,"Distanzenkraft");
        InsertTaste(1,"Zeitenkraft");
}

void Billard::Reset(){
        Clear(Schwarz);
	DrawTable();
	InitQueue();

        for(int i = 0; i < N; i++) {
		kugeln[i].init_Kugel(i, kugeln_X[i], kugeln_Y[i], colors[i]);
	}
	DrawKugeln();

        Mode = 0;
        K0On = 0;
        K0Move = 1;
}

void Billard::RunTaste0(){ Mode = 0;}

void Billard::RunTaste1(){ Mode = 1;}

void Billard::DrawTable(){
	Clear();
	SetPen(Schwarz);
	z=RGBSkala(50.0,30.0,0.0);
	SetBrush(z);
	Rectangle(-1405.0,-765.0,2810.0,1530.0);
	Rectangle(-1405.0,-865.0,2810.0,100.0);

        Rectangle(-1405.0,765.0,2810.0,100.0);
        
	SetPen(Schwarz,2);
	SetBrush(Gruen);

	//Rectangle(-1270.0,-635.0,2540.0,1270.0); // Spielfeld (ohne Bande)
        Rectangle(-1250.0,-615.0,2500.0,1230.0); // Spielfeld (ohne Bande)

	SetPen(Grau);
	SetBrush(Grau);
	Rectangle(-400.0,-845.0,800.0,60.0);
	Circle(-400.0,-815.0,30.0);
	Circle(400.0,-815.0,30.0);
	SetPen(Schwarz,1);
	SetBrush(Schwarz);
      /*	Circle(-1270.0,-635.0,50.0);  // Löcher
	Circle(-1270.0,635.0,50.0);
	Circle(1270.0,-635.0,50.0);
	Circle(1270.0,635.0,50.0);
	Circle(0.0,-635.0,50.0);
	Circle(0.0,635.0,50.0); // Löcher Ende  */

        Circle(-1210.0,-575.0,60.0);// Löcher
        Circle(-1210.0,575.0,60.0);
        Circle(1210.0,-575.0,60.0);
        Circle(1210.0,575.0,60.0);
        Circle(0.0,-600.0,60.0);
        Circle(0.0,600.0,60.0);// Löcher Ende  */

	SetPen(Schwarz,2);
	MoveTo(-1220.0,-635.0); // Bande
	LineTo(-1180.0,-595.0);
        LineTo(-50.0,-595.0);
	//LineTo(-90.0,-595.0);
	//LineTo(-50.0,-635.0);
	MoveTo(1220.0,-635.0);
	LineTo(1180.0,-595.0);
        LineTo(50.0,-595.0);
	//LineTo(90.0,-595.0);
	//LineTo(50.0,-635.0);
	MoveTo(1220.0,635.0);
	LineTo(1180.0,595.0);
        LineTo(50.0,595.0);
	//LineTo(90.0,595.0);
	//LineTo(50.0,635.0);
	MoveTo(-1220.0,635.0);
	LineTo(-1180.0,595.0);
        LineTo(-50.0,595.0);
	//LineTo(-90.0,595.0);
	//LineTo(-50.0,635.0);
	MoveTo(-1270.0,-585.0);
	LineTo(-1230.0,-545.0);
	LineTo(-1230.0,545.0);
	//LineTo(-1270.0,585.0);
	MoveTo(1270.0,-585.0);
	LineTo(1230.0,-545.0);
	LineTo(1230.0,545.0);
	//LineTo(1270.0,585.0);
	SetPen(Schwarz,1);
	MoveTo(-635.0,595.0);
	LineTo(-635.0,-595.0);
	SetPen(Weiss);
	SetBrush(Weiss);
      /*	Circle(-300.0,700.0,10.0);
	Circle(-600.0,700.0,10.0);
	Circle(-900.0,700.0,10.0);
	Circle(300.0,700.0,10.0);
	Circle(600.0,700.0,10.0);
	Circle(900.0,700.0,10.0);
	Circle(-300.0,-700.0,10.0);
	Circle(-600.0,-700.0,10.0);
	Circle(-900.0,-700.0,10.0);
	Circle(300.0,-700.0,10.0);
	Circle(600.0,-700.0,10.0);
	Circle(900.0,-700.0,10.0);
	Circle(-1337.5,0.0,10.0);
	Circle(-1337.5,350.0,10.0);
	Circle(-1337.5,-350.0,10.0);
	Circle(1337.5,0.0,10.0);
	Circle(1337.5,350.0,10.0);
	Circle(1337.5,-350.0,10.0); */
        Circle(-300.0,680.0,10.0);
	Circle(-600.0,680.0,10.0);
	Circle(-900.0,680.0,10.0);
	Circle(300.0,680.0,10.0);
	Circle(600.0,680.0,10.0);
	Circle(900.0,680.0,10.0);
	Circle(-300.0,-680.0,10.0);
	Circle(-600.0,-680.0,10.0);
	Circle(-900.0,-680.0,10.0);
	Circle(300.0,-680.0,10.0);
	Circle(600.0,-680.0,10.0);
	Circle(900.0,-680.0,10.0);
	Circle(-1320.0,0.0,10.0);
	Circle(-1320.0,350.0,10.0);
	Circle(-1320.0,-350.0,10.0);
	Circle(1320.0,0.0,10.0);
	Circle(1320.0,350.0,10.0);
	Circle(1320.0,-350.0,10.0);
}

void Billard::DrawKugeln() {
	double x, y;
	int n;
	TColor color; 
	PlanString label;
	for(int i = 0; i < N; i++) {
		n = kugeln[i].n;
		x = kugeln[i].pos[0];
		y = kugeln[i].pos[1];
		label = kugeln[i].n;
		color = kugeln[i].color;
		if( ! n ) {
			SetPen(Schwarz, 2);
			SetBrushColor(color);
			Circle(x,y,r);
		}
		else if( n > 8 ) {
			SetPen(Weiss);
			SetBrush(Weiss);
			Circle(x,y,r);
			SetPenColor(color);
			SetBrushColor(color);
			View(x-r, y-r*0.7, 2*r, 1.4*r);
			Scale(x-r, x+r, y - r*0.7, y+0.7*r);
			Circle(x,y,r);
			Text(x-0.5*r,y+0.5*r,label);

			View();
			Scale(-1700.0,1700.0,0.0);
			y = IntToY(0);
			Scale(-1700.0,1700.0,-y/2.0);
			SetPen(Schwarz,2);
			SetBrush(Klar);
			Circle(x,y,r);
		}
		else {
			SetPen(Schwarz, 2);
			SetBrushColor(color);
			Circle(x,y,r);
			Text(x-0.5*r,y+0.5*r,label);
		}
	}
}

void Billard::Stoss() {
	moving = true;
	//log(force);
	//vlog(ForceScale * force * ( Z - kugeln[0].pos)/norm( Z - kugeln[0].pos));
	kugeln[0].v = ForceScale * force * ( Z - kugeln[0].pos)/norm( Z - kugeln[0].pos);
}

bool Billard::CheckMoving() {
	bool m = false;
	for(int i = 0; i < N; i++) {
		if ( MaxNorm(kugeln[i].v) > 0.1) m = true;
	}
	return m;
}

void Billard::CheckHoles() {
        for(int i = 0; i < N; i++) {
	    if(dist(kugeln[i].pos[0], kugeln[i].pos[1], -1210.0, -575.0) < 60.0||
               dist(kugeln[i].pos[0], kugeln[i].pos[1], -1210.0, 575.0) < 60.0||
               dist(kugeln[i].pos[0], kugeln[i].pos[1], 1210.0, -575.0) < 60.0||
               dist(kugeln[i].pos[0], kugeln[i].pos[1], 1210.0, 575.0) < 60.0||
               dist(kugeln[i].pos[0], kugeln[i].pos[1], 0.0, -600.0) < 60.0||
               dist(kugeln[i].pos[0], kugeln[i].pos[1], 0.0, 600.0) < 60.0){
                        kugeln[i].pos = TVektor(i*100.0-800.0,800.0);
                        kugeln[i].v = TVektor(0.01,0.01);
                } }
        if(kugeln[0].pos[1] == 800.0)    {kugeln[0].v=TVektor(0.0,0.0); kugeln[0].pos = TVektor(-635.0,0.0);  }
        K0[0] = kugeln[0].pos[0];
}

void Billard::HandleBoxCollision() {
	double tcx;	// Zeitanteil in dem die Kollision stattfinden würde in x
	double tcy;	// Zeitanteil in dem die Kollision stattfinden würde in y
	for(int i = 0; i < N; i++) {
		kugeln[i].next = kugeln[i].pos + kugeln[i].v * (1 - my);

		//CheckHoles(i);
                if(kugeln[i].next[1]<650){
		if ( kugeln[i].next[0] - r < -1230.0 ) {
			tcx = (-1230.0 + r - kugeln[i].next[0])/(kugeln[i].pos[0] - kugeln[i].next[0]);
			kugeln[i].v[0] *= - bande;
			kugeln[i].next[0] = -1230.0 + r + kugeln[i].v[0] * (1 - my) * (1-tcx);
		}
		else if ( kugeln[i].next[0] + r > 1230.0 ) {
			tcx = ( 1230.0 - r - kugeln[i].next[0])/(kugeln[i].pos[0] - kugeln[i].next[0]);
			kugeln[i].v[0] *= - bande;
			kugeln[i].next[0] = 1230.0 - r + kugeln[i].v[0] * (1 - my) * (1-tcx);
		}
		if ( kugeln[i].next[1] - r <  -600.0 ) {
			tcy = ( -600.0 + r - kugeln[i].next[1])/(kugeln[i].pos[1] - kugeln[i].next[1]);
			kugeln[i].v[1] *= - bande;
			kugeln[i].next[1] = - 600.0 + r + kugeln[i].v[1] * (1 - my) * (1-tcy);
		}
		else if ( kugeln[i].next[1] + r > 600.0 ) {
			tcy = (  600.0 - r - kugeln[i].next[1])/(kugeln[i].pos[1] - kugeln[i].next[1]);
			kugeln[i].v[1] *= - bande;
			kugeln[i].next[1] = 600.0 - r + kugeln[i].v[1] * (1 - my) * (1-tcy);
		}}
                else kugeln[i].v = TVektor(0.0,0.0);
	}
}


double distKugeln(Kugel k1, Kugel k2) {
	return sqrt(pow(k1.pos[0] - k2.pos[0],2) + pow(k1.pos[1] - k2.pos[1],2));
}

void Billard::BallCCD(int i) {
}

void Billard::HandleBallCollision() {
	TVektor vn;
	double delta, dist;
	for(int k = 0; k < N; k++) { kugeln[k].next = kugeln[k].pos + kugeln[k].v * (1 - my); }
	for(int i = 0; i < N; i++) {
		if( 30.0 < norm(kugeln[i].v)) {
			BallCCD(i);
		}
		for(int j = i+1; j < N; j++) {
			dist = distKugeln(kugeln[i],kugeln[j]);
			if(2*r >= dist) {
				vn = (kugeln[j].pos - kugeln[i].pos)/norm(kugeln[j].pos - kugeln[i].pos);
				if(2*r > dist) {
					kugeln[i].pos -= (2*r - dist) * vn;
					kugeln[j].pos += (2*r - dist) * vn;
				}
				delta = dot(vn, kugeln[j].v) - dot(vn, kugeln[i].v);
				kugeln[i].v += delta * vn;
				kugeln[j].v -= delta * vn;
			}
		}
	}
}

#pragma argsused
int main(int argc, char* argv[]) {
	Billard billard;
	billard.Execute();
	return 0;
}
