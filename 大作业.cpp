#include <easyx.h>
#include <stdio.h>
#include <graphics.h>
#include<time.h>
#include"tools.h"
#include"vector2.h"
#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")
struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;
	int frameIndex;
};
IMAGE imgBulletBlast[4];
struct bullet bullets[30];
IMAGE imgBulletNormal;
struct zm{
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;
	int blood;
	bool dead;
	bool eating;//是否正在吃植物
};
IMAGE imgZmDead[20];
IMAGE imgzmeat[21];
struct zm zms[10];
IMAGE imgZM[22];
struct zhiwu {
	int type;
	int frameindex;
	bool catched;//是否被僵尸捕获
	int deadtime;//死亡计数器
	int timer;
	int x, y;

};
struct zhiwu map[3][9];
enum{SUNSHINE_DOWN,SUNSHINE_GROUND,SUNSHINE_COLLECT,SUNSHINE_PRODUCT};
struct sunshineball {
	int x, y;
	int frameindex;//当前图片帧的序号
	int destY;//目标位置的纵坐标
	int used;//是否在使用
	int timer;
	double xoff;
	double yoff;
	double t;
	vector2 p1, p2, p3, p4;
	vector2 pcur;
	double speed;
	int status;
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
void drawsunshine() {
	int ballmax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballmax; i++) {
		if (balls[i].used) {
			IMAGE* img = &imgsunshineball[balls[i].frameindex];
			putimagePNG(balls[i].pcur.x, balls[i].pcur.y, img);
		}
	}
}
void checkbullet2zm() {
	int bCount = sizeof(bullets) / sizeof(bullets[0]);
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < bCount; i++) {
		if (bullets[i].used == false || bullets[i].blast)continue;
		for (int k = 0; k < zCount; k++) {
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			if (zms[k].dead == false &&  //添加这个条件
				bullets[i].row == zms[k].row && bullets[i].x > x1 && bullets[i].x < x2) {
				zms[k].blood -= 5;
				bullets[i].blast = true;
				bullets[i].speed = 0;

				//对血量进行检测
				if (zms[k].blood <= 0) {
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;
				}
				break;
			}
		}
	}
}
void checkzm2zhiwu() {
	int zcount = sizeof(zms)/sizeof(zms[0]);
	for (int i = 0; i < zcount; i++) {
		if (zms[i].dead) { continue; }
		int row = zms[i].row;
		for (int k = 0; k < 9; k++) {
			if (map[row][k].type == 0) { continue; }
			int zhiwux = 256 + k * 81;
			int x1 = zhiwux + 10;
			int x2 = zhiwux + 60;
			int x3 = zms[i].x + 80;
			if (x3 > x1 && x3 < x2) {
				if (map[row][k].catched) {
					map[row][k].deadtime++;
					if (map[row][k].deadtime >= 100) {
						map[row][k].deadtime = 0;
						map[row][k].type = 0;
						zms[i].eating = false;
						zms[i].frameIndex = 0;
						zms[i].speed = 1;
					}
				}
				else { 
					map[row][k].catched = true;
					map[row][k].deadtime = 0;
					zms[i].eating = true;
					zms[i].speed = 0;
					zms[i].frameIndex = 0;
				}
			}

		}
	}
}
void collisionCheck() {
	checkbullet2zm();
	checkzm2zhiwu();
}
void drawZM() {
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used) {
			//选择对应的渲染图片
			IMAGE *img = NULL;
			if (zms[i].dead)img = imgZmDead;
			else if (zms[i].eating) { img = imgzmeat; }
			else img = imgZM;


			img += zms[i].frameIndex;

			int x = zms[i].x;
			int y = zms[i].y - img->getheight();
			putimagePNG(x, y, img);
		}
	}
}
// 发射接口
void shoot() {
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	int directions[3] = { 0 };
	int dangerX = WIDTH - imgZM[0].getwidth();
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used && zms[i].x < dangerX) {
			directions[zms[i].row] = 1;
		}
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == wandou + 1 && directions[i]) {
				static int count = 0;
				count++;
				if (count > 20) {
					count = 0;
					int k;
					int maxCount = sizeof(bullets) / sizeof(bullets[0]);
					for (k = 0; k < maxCount && bullets[k].used; k++);
					if (k < maxCount) {
						bullets[k].row = i;
						bullets[k].speed = 4;
						bullets[k].used = true;
						bullets[k].blast = false;
						bullets[k].frameIndex = 0;

						int zwX = 260 + j * 81.6;    // (msg.x - 260) / 81.6;
						int zwY = 180 + i * 103.6 + 14; // (msg.y - 210) / 103.6;

						bullets[k].x = zwX + imgzhiwu[map[i][j].type - 1][0]->getwidth() - 10;
						bullets[k].y = zwY + 5;
					}
				}
			}
		}
	}
}
//更新子弹的状态
void updateBullets() {
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIDTH) {
				bullets[i].used = false;
			}
		}
	

	if (bullets[i].blast) {
		bullets[i].frameIndex++;
		if (bullets[i].frameIndex >= 4) {
			bullets[i].used = false;
		}
	}
  }
}
void updatesunshine() {
	int ballmax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballmax; i++) {
		if (balls[i].used) {
			balls[i].frameindex = (balls[i].frameindex + 1) % 29;
			if (balls[i].status == SUNSHINE_DOWN) {
				struct sunshineball* sun = &balls[i];
				sun->t += sun->speed;
				sun->pcur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND) {
				balls[i].timer++;
				if (balls[i].timer > 300) {
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT) {
				struct sunshineball* sun = &balls[i];
				sun->t += sun->speed;
				sun->pcur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t > 1) {
					sun->used = false;
					
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCT) {
				struct sunshineball* sun = &balls[i];
				sun->t += sun->speed;
				sun->pcur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
					
				}
			}
		}
	}
	/*for (int i = 0; i < ballmax; i++) {
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

	}*/


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
		//初始化僵尸
		memset(zms, 0, sizeof(zms));
		srand(time(NULL));

		for (int i = 0; i < 22; i++) {
			sprintf_s(name, sizeof(name), "res/zm/%d.png", i + 1);
			loadimage(&imgZM[i], name);
		}
		loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
		memset(bullets, 0, sizeof(bullets));
		loadimage(&imgBulletBlast[3], "res/bullets/bullet_blast.png");
		for (int i = 0; i < 3; i++) {
			float k = (i + 1) * 0.2;
			loadimage(&imgBulletBlast[i], "res/bullets/bullet_blast.png",
				imgBulletBlast[3].getwidth() * k,
				imgBulletBlast[3].getheight() * k, true);
		}
		for (int i = 0; i < 20; i++) {
			sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
			loadimage(&imgZmDead[i], name);
		}
		for (int i = 0; i < 21; i++) {
			sprintf_s(name, "res/zm_eat/%d.png", i + 1);
			loadimage(&imgzmeat[i], name);
		}

}
//创建僵尸
void createZM() {
	static int zmFre = 500;
	static int count = 0;
	count++;
	if (count > zmFre) {
		zmFre = rand() % 200 + 300;
		count = 0;

		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMax && zms[i].used; i++);
		if (i < zmMax) {
			zms[i].used = true;
			zms[i].x = WIDTH;
			zms[i].y = 180 + (1 + rand() % 3) * 100 - 8;
			zms[i].speed = 1;
			zms[i].blood = 100;
			zms[i].dead = false;
			if (i < zmMax) {
				memset(&zms[i], 0, sizeof(zms[i]));
				zms[i].used = true;
				zms[i].x = WIDTH;

				zms[i].row = rand() % 3; // 0..2;
				zms[i].y = 172 + (1 + zms[i].row) * 100;

				zms[i].speed = 1;
				zms[i].blood = 100;
			}
		}
	}
}
//绘制僵尸的接口

//更新僵尸
void updateZM() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	static int count1 = 0;
	count1++;
	if (count1 > 2) {
		count1 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms->x < 236 - 66) {
					printf("GAME OVER!\n");
					MessageBox(NULL, "over", "over", 0); //TO DO
					break;
				}
			}
		}
	}

	static int count2 = 0;
	count2++;
	if (count2 > 4) {
		count2 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				//判断是否已经死亡
				if (zms[i].dead) {
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20) {
						zms[i].used = false;
					}
				}
				else if (zms[i].eating) {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 21;
				}
				else {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				}
			}
		}
	}
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
				
				int zhiwutype = map[i][j].type - 1;
				int index = map[i][j].frameindex;
				putimagePNG(map[i][j].x, map[i][j].y, imgzhiwu[zhiwutype][index]);
			}
		}
	}
	if (curzhiwu > 0) {
		IMAGE* img = imgzhiwu[curzhiwu - 1][0];
		putimagePNG(curx-img->getwidth()/2, cury-img->getheight()/2, img);
	}
	drawsunshine();
		char scoreText[8];
		sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
		outtextxy(276, 67, scoreText);
		drawZM();

		int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
		for (int i = 0; i < bulletMax; i++) {
			if (bullets[i].used) {
				if (bullets[i].blast) {
					IMAGE* img = &imgBulletBlast[bullets[i].frameIndex];
					int x = bullets[i].x + 12 - img->getwidth() / 2;
					int y = bullets[i].y + 12 - img->getheight() / 2;
					putimagePNG(x, y, img);

					/*bullets[i].used = false;*/
				}
				else {
					putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
				}

			}
		}
	EndBatchDraw();//结束双缓冲
}
void collectsunshine(ExMessage*msg) {
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgsunshineball[0].getwidth();
	int h = imgsunshineball[0].getheight();
	for (int i = 0; i < count; i++) {
		if (balls[i].used) {
			int x = balls[i].pcur.x;
			int y = balls[i].pcur.y;
			if (msg->x > x && msg->x<x + w && msg->y>y && msg->y < y + h) {
				//balls[i].used = false;
				balls[i].status = SUNSHINE_COLLECT;
				sunshine += 25;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);
				balls[i].p1 = balls[i].pcur;
				balls[i].p4 = vector2(262, 0);
					balls[i].t = 0;
					double distance = dis(balls[i].p1 - balls[i].p4);
					double off = 8;
					balls[i].speed = 1 / (distance / off);
					
					break;
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
					if (curzhiwu == wandou + 1) {
						sunshine -= 100;
						
					}
					else if (curzhiwu == xiangrikui + 1) {
						sunshine -= 50;
						
					}
					map[row][col].type = curzhiwu;
					map[row][col].frameindex = 0;
					map[row][col].x = 256 + col * 81;
					map[row][col].y = 179 + row * 102 + 14;
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
		//balls[i].x = 260 + rand() % (900 - 260);
		//balls[i].destY = 200 + (rand() % 4) * 90;
		balls[i].timer = 0;
		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 + rand() % (900 - 260) , 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		double distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1 / (distance / off);
	}
	//从阳光池中选择可以使用的
	//下面是向日葵生产阳光
	int ballmax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == xiangrikui+1) {
				map[i][j].timer++;
				if (map[i][j].timer > 200) {
					map[i][j].timer = 0;
					int k;
					for ( k = 0; k < ballmax && balls[k].used; k++) {
						if (k >= ballmax) { return; }
						balls[k].used = true;
						balls[k].p1 = vector2(map[i][j].x, map[i][j].y);
						int w = (100 + rand() % 50) * (rand() % 2 ? 1 : -1);
						balls[k].p4 = vector2(map[i][j].x + w, map[i][j].y + imgzhiwu[xiangrikui][0]->getheight() - imgsunshineball[0].getheight());
						balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);
						balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);
						balls[k].status = SUNSHINE_PRODUCT;
						balls[k].speed = 0.05;
						balls[k].t = 0;
					}
				}
			}
		}
	}
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
	creatsunshine();//阳光
	updatesunshine();
	createZM();//僵尸
	updateZM();

	shoot();//子弹
	updateBullets();
	collisionCheck();//实现子弹爆炸的检测
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