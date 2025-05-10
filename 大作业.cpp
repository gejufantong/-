#include <easyx.h>
#include <stdio.h>
#include <graphics.h>
#include<time.h>
#include"tools.h"
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")
struct zhiwu {
	int type;
	int frameindex;
};
struct zhiwu map[3][9];
struct sunshineball {
	int x, y;
	int frameindex;//当前图片帧的序号
	int destY;//目标位置的纵坐标
	int used;//是否在使用
	int timer;
};
struct sunshineball balls[10];
IMAGE imgsunshineball[29];
int sunshine;
#define WIDTH 1000
#define HEIGHT 600
enum{wandou,xiangrikui,zhiwucount};
IMAGE imgbg;//表示背景图片
IMAGE imgbar;//阳光栏
IMAGE imgcards[zhiwucount];
IMAGE *imgzhiwu[zhiwucount][20];
int curx, cury;//鼠标当前的x,y坐标
int curzhiwu;//0表示没有选中植物，1表示选中第一种植物；
bool fileexist(const char* name) {
	FILE* fp = fopen(name, "r");
	if (fp == NULL) {
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}
void updatesunshine() {
	int ballmax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballmax; i++) {
		if (balls[i].used) {
			balls[i].frameindex = (balls[i].frameindex + 1) % 29;
			if (balls[i].timer == 0) { balls[i].y += 2; }

			if (balls[i].y >= balls[i].destY) {
				
				balls[i].timer++;
				if (balls[i].timer >= 100) {
					balls[i].used = false;
				}
			}
		}

	}


}
void gameInit() {
	//
	loadimage(&imgbg, "res/bg.jpg");
	//创建游戏窗口
	loadimage(&imgbar, "res/bar5.png");
	//初始化植物
	memset(imgzhiwu, 0, sizeof(imgzhiwu));
	memset(map, 0, sizeof(map));
	char name[64];
	for (int i = 0; i < zhiwucount; i++) {
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1); 
		loadimage(&imgcards[i], name);
		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i ,j+1);
			if (fileexist(name)) {
				imgzhiwu[i][j] = new IMAGE;
				loadimage(imgzhiwu[i][j], name);
			}
			else {
				break;
			}
		}
	}
	curzhiwu = 0;
	sunshine = 50;
	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgsunshineball[i], name);
	}
	srand(time(NULL));
	initgraph(WIDTH, HEIGHT,1);
	LOGFONT f;
	gettextstyle(&f);
		f.lfHeight = 30;
		f.lfWeight = 15;
		strcpy(f.lfFaceName, "Segoe UI BLACK");
		settextstyle(&f);
		setbkmode(TRANSPARENT);
		setcolor(BLACK);
}
void updatewindow() {
	BeginBatchDraw();
	putimage(0, 0, &imgbg);
	putimagePNG(250, 0, &imgbar);
	for (int i = 0; i < zhiwucount; i++) {
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgcards[i]);
	}
	
	//渲染拖动过程中的植物
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type > 0) {
				int x = 256 + j * 81;
				int y = 179 + i * 102;
				int zhiwutype = map[i][j].type - 1;
				int index = map[i][j].frameindex;
				putimagePNG(x, y, imgzhiwu[zhiwutype][index]);
			}
		}
	}
	if (curzhiwu > 0) {
		IMAGE* img = imgzhiwu[curzhiwu - 1][0];
		putimagePNG(curx-img->getwidth()/2, cury-img->getheight()/2, img);
	}
	int ballmax = sizeof(balls) / sizeof(balls[0]);
		for (int i = 0; i < ballmax; i++) {
			if (balls[i].used) {
				IMAGE* img = &imgsunshineball[balls[i].frameindex];
				putimagePNG(balls[i].x, balls[i].y, img);
			}
		}
		char scoreText[8];
		sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
		outtextxy(276, 67, scoreText);
	EndBatchDraw();//结束双缓冲
}
void collectsunshine(ExMessage*msg) {
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgsunshineball[0].getwidth();
	int h = imgsunshineball[0].getheight();
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			int x = balls[i].x;
			int y = balls[i].y;
			if (msg->x > x && msg->x<x + w && msg->y>y && msg->y < y + h) {
				balls[i].used = false;
				sunshine += 25;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);
			}
		}
	}
}
void userClick() {
	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 338 && msg.x < 338 + 65 * zhiwucount && msg.y < 96) {
				int index = (msg.x - 338) / 65;
				status = 1;
				curzhiwu = index + 1;
			}
			else {
				collectsunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE&&status ==1) {
			curx = msg.x;
			cury = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {
			if (msg.x > 256 && msg.y > 179 && msg.y < 479) {
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;
				if (map[row][col].type == 0) {
					map[row][col].type = curzhiwu;
					map[row][col].frameindex = 0;
				}
				
			}
			curzhiwu = 0;
			status = 0;
		}

	}
}
void creatsunshine() {
	static int count = 0;
	static int fre = 400;
	count++;
	if (count >= fre) {
		fre = 200 + rand() % 200;
		count = 0;
		int ballmax = sizeof(balls) / sizeof(balls[0]);
		int i;
		for (i = 0; i < ballmax && balls[i].used; i++)
			if (i >= ballmax) { return; }
		balls[i].used = true;
		balls[i].frameindex = 0;
		balls[i].x = 260 + rand() % (900 - 260);
		balls[i].destY = 200 + (rand() % 4) * 90;
		balls[i].timer = 0;
	}
	//从阳光池中选择可以使用的
}
void updategame() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j] .type> 0) {
				map[i][j].frameindex++;
				int zhiwutype = map[i][j].type - 1;
				int index1 = map[i][j].frameindex;
				if (imgzhiwu[zhiwutype][index1] == NULL) {
					map[i][j].frameindex = 0;
				}
			}
		}
	}
	creatsunshine();
	updatesunshine();
}
//显示开始菜单
void startui() {
	//加载开始菜单
	IMAGE imgbg, imgmenu1, imgmenu2;
	loadimage(&imgbg, "res/menu.png");
	loadimage(&imgmenu1, "res/menu1.png");
	loadimage(&imgmenu2, "res/menu2.png");
	int flag = 0;
	//以下为点击后进入游戏界面
	while (1) {
		BeginBatchDraw();
		putimage(0, 0, &imgbg);
		putimagePNG(474, 75, flag ? &imgmenu2 : &imgmenu1);
		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN && msg.x > 474 && msg.x < 474 + 300 && msg.y>75 && msg.y < 75 + 140) {
				flag = 1;
				EndBatchDraw();
			}
			else if (msg.message == WM_LBUTTONUP && flag) {
				return;
			}
		}
		EndBatchDraw();


	}

}



int main() {
	gameInit();
	startui();
	int timer = 0;
	bool flag = true;
	while (1) {
		userClick();
		timer += getDelay();
		if (timer >= 20) {
			flag = true;
			timer = 0;
		}
		if (flag) {
			flag = false;
			updatewindow();
			updategame();
			
		}
	}
	system("pause");
	return 0;
}