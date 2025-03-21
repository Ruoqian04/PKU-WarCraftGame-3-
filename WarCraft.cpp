#include<iostream>
#include<iomanip>
#include<vector>
#include<string>
#include<algorithm>
#include<array>

using namespace std;

bool End = false;//游戏是否截止
bool Rpd = false;//汇报过了
int hour = 0;
int minute = 0;
int K;//每走一步就下降K
int N;//之间有N个城市
int hps[5] = { 0 };//每种武士对应的hp
int aggrs[5] = { 0 };
array<string, 3> Weapons = { "sword", "bomb", "arrow" };
array<string, 5> Warriors = { "dragon","ninja","iceman","lion","wolf" };

class Warrior;

class City {
public:
    int num = 0;//编号
    int sum = 0;
    Warrior* redW = nullptr;
    Warrior* blueW = nullptr;
}cities[22];

class Weapon {
public:
    int kind;//武器编号 即种类 0 1 2
    int aggr;//攻击力
    int times = 1;

    Weapon(int k, int a) :kind(k), aggr(a) {
        if (k == 0) aggr = int(aggr * 2 / 10);
        if (k == 1) aggr = int(aggr * 4 / 10);
        if (k == 2) {
            times = 2;
            aggr = int(aggr * 3 / 10);
        }
    };

    void refreshAggr(int wAggr) {
        if (kind == 0) aggr = int(wAggr * 2 / 10);
        if (kind == 1) aggr = int(wAggr * 4 / 10);
        if (kind == 2) aggr = int(wAggr * 3 / 10);
    }

};

bool compare(Weapon* a, const Weapon* b) {//战斗之前对武器进行排序
    if (a->kind == 2 && b->kind == 2) return a->times < b->times;
    return a->kind < b->kind;
}

bool compareAfterfight(Weapon* a, Weapon* b) {//缴获武器时对武器进行排序
    if (a->kind == 2 && b->kind == 2) return a->times > b->times;
    return a->kind < b->kind;
}

class Warrior {
public:
    int hp;//生命值
    int num;//对应的编号
    int aggr;//攻击力
    int kind;//武士的种类 0~4
    int color;//1为红色 0为蓝色
    int location = (color ? 0 : N + 1);//所在城市 0为红色司令部 N+1为蓝色
    int goal = (color ? N + 1 : 0);//敌方司令部
    bool isDead = false;//判断是否死亡
    vector<Weapon*> weapons;
    bool isNewWar = true;//是否是一场新的战斗
    int idx = 0;//用武器的顺序

    Warrior(int m, int n, int a, int k, int c) :hp(m), num(n), aggr(a), kind(k), color(c) {}

    virtual void Runaway(City& city) {};
    virtual void beforeWar(Warrior* w) {};
    virtual void yell(City& city) {};

    void reportSelf() {//武士汇报
        int ws[3] = { 0 };

        for (int i = 0;i < weapons.size();i++) {
            ws[weapons[i]->kind]++;
        }
        if (color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red " << Warriors[kind] << ' ' << num << " has " << ws[0] << " sword " << ws[1] << " bomb " << ws[2] << " arrow and " << hp << " elements" << endl;
        else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue " << Warriors[kind] << ' ' << num << " has " << ws[0] << " sword " << ws[1] << " bomb " << ws[2] << " arrow and " << hp << " elements" << endl;
    }

    virtual int useWeapon() {//使用武器 无武器或者死亡都无法使用
        int l = weapons.size();
        if (l == 0 || hp <= 0) return 0;
        if (isNewWar) {
            sort(weapons.begin(), weapons.end(), compare);
            idx = 0;
            isNewWar = false;
        }

        int attack = weapons[idx]->aggr;
        
        if (weapons[idx]->kind == 1) {//bomb用完要伤及自身
            getHurt(int(attack / 2));
            Weapon* wp = weapons[idx];
            weapons.erase(weapons.begin() + idx);
            delete wp;
            if (weapons.size() > 0) idx %= weapons.size(); // 调整idx，避免越界
        }

        else if (weapons[idx]->kind == 2) {//arrow用完清出内存
            weapons[idx]->times -= 1;
            if (weapons[idx]->times == 0) {
                Weapon* wp = weapons[idx];
                weapons.erase(weapons.begin() + idx);
                delete wp;
                if (weapons.size() > 0) idx %= weapons.size(); // 调整idx
            }
            else {
                idx = (idx + 1) % weapons.size();
            }
        }
        else {
            idx = (idx + 1) % weapons.size();
        }

        return attack;
    }

    void getWeapon(Warrior* w) {//战后缴获武器
        int l = w->weapons.size();
        if (l == 0 || weapons.size() >= 10) return;

        sort(w->weapons.begin(), w->weapons.end(), compare);//从小到大缴获
        for (auto it = w->weapons.begin(); it != w->weapons.end();) {
            if (weapons.size() >= 10) break;
            else {
                (*it)->refreshAggr(aggr);//武器重置攻击力
                weapons.push_back((*it));
                it = w->weapons.erase(it);
            }
        }

    };

    virtual void Forward(City& now, City& next) {//前进
        if (color) {
            location += 1;
            now.redW = nullptr;
            next.redW = this;
            if (location == goal) End = true;
        }
        else {
            location -= 1;
            now.blueW = nullptr;
            next.blueW = this;
            if (location == goal) End = true;
        }
        now.sum--;
        next.sum++;
    }

    void tellWhere(City& city) {//汇报位置
        if (city.num != 0 && city.num != N + 1) {
            if (color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red " << Warriors[kind] << ' ' << num << " marched to city " << city.num << " with " << hp << " elements and force " << aggr << endl;
            else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue " << Warriors[kind] << ' ' << num << " marched to city " << city.num << " with " << hp << " elements and force " << aggr << endl;
        }
        else {
            if (color) {
                cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red " << Warriors[kind] << ' ' << num << " reached blue headquarter with " << hp << " elements and force " << aggr << endl;
                cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue headquarter was taken" << endl;

            }
            else {
                cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue " << Warriors[kind] << ' ' << num << " reached red headquarter with " << hp << " elements and force " << aggr << endl;
                cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red headquarter was taken" << endl;
            }
            End = true;

            return;
        }
    }

    void getHurt(int attack) {//检查受伤后是否死亡
        hp -= attack;
        if (hp <= 0) {
            isDead = true;
            aggr = 0;
        }
    }

    virtual ~Warrior() {
        for (int i = 0;i < weapons.size();i++) {
            delete weapons[i];
        }
    };
};

class Dragon :public Warrior {
public:
    Dragon(int m, int n, int c) :Warrior(m, n, aggrs[0], 0, c) {
        weapons.push_back(new Weapon(n % 3, aggrs[0]));
        if (color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red dragon " << num << " born" << endl;
        else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue dragon " << num << " born" << endl;
    }

    void yell(City& city) override {
        if (hp <= 0) return;
        if (color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red dragon " << num << " yelled in city " << city.num << endl;
        else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue dragon " << num << " yelled in city " << city.num << endl;
    }
};

class Ninja :public Warrior {
public:
    Ninja(int m, int n, int c) :Warrior(m, n, aggrs[1], 1, c) {
        weapons.push_back(new Weapon(n % 3, aggrs[1]));
        weapons.push_back(new Weapon((n + 1) % 3, aggrs[1]));
        if (color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red ninja " << num << " born" << endl;
        else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue ninja " << num << " born" << endl;
    }

    int useWeapon() override {
        int l = weapons.size();
        if (l == 0) return 0;
        if (isNewWar) {
            sort(weapons.begin(), weapons.end(), compare);
            isNewWar = false;
        }

        int attack = weapons[idx]->aggr;

        if (weapons[idx]->kind == 1) {//bomb用完就没
            Weapon* wp = weapons[idx];
            weapons.erase(weapons.begin() + idx);
            delete wp;
            if (weapons.size() > 0) idx %= weapons.size(); // 调整idx，避免越界
        }
        else if (weapons[idx]->kind == 2) {//arrow用完清出内存
            weapons[idx]->times -= 1;
            if (weapons[idx]->times == 0) {
                Weapon* wp = weapons[idx];
                weapons.erase(weapons.begin() + idx);
                delete wp;
                if (weapons.size() > 0) idx %= weapons.size(); // 调整idx
            }
            else {
                idx = (idx + 1) % weapons.size();
            }
        }
        else {
            idx = (idx + 1) % weapons.size();
        }

        return attack;
    }
};

class Iceman :public Warrior {
public:
    Iceman(int m, int n, int c) :Warrior(m, n, aggrs[2], 2, c) {
        weapons.push_back(new Weapon(n % 3, aggrs[2]));
        if (color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red iceman " << num << " born" << endl;
        else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue iceman " << num << " born" << endl;
    }

    void Forward(City& now, City& next) override {
        if (color) {
            location += 1;
            now.redW = nullptr;
            next.redW = this;
            if (location == goal) End = true;
        }
        else {
            location -= 1;
            now.blueW = nullptr;
            next.blueW = this;
            if (location == goal) End = true;
        }
        hp -= int(hp / 10);
        if (hp <= 0) isDead = true;
        now.sum--;
        next.sum++;
    }
};

class Lion :public Warrior {
public:
    int loyalty;

    Lion(int m, int n, int c, int lo) :Warrior(m, n, aggrs[3], 3, c), loyalty(lo) {
        weapons.push_back(new Weapon(n % 3, aggrs[3]));
        if (color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red lion " << num << " born" << endl;
        else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue lion " << num << " born" << endl;
        cout << "Its loyalty is " << loyalty << endl;
    }

    void Runaway(City& city) override {
        if (loyalty <= 0 && location != goal) {
            hp = 0;
            isDead = true;
            city.sum--;
            if (color) {
                city.redW = nullptr;
                cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red lion " << num << " ran away" << endl;
            }
            else {
                city.blueW = nullptr;
                cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue lion " << num << " ran away" << endl;
            }
        }
    }

    void Forward(City& now, City& next) override {
        if (color) {
            location += 1;
            now.redW = nullptr;
            next.redW = this;
        }
        else {
            location -= 1;
            now.blueW = nullptr;
            next.blueW = this;
        }
        if (location == goal) End = true;
        if (loyalty <= 0 && next.num != goal) {
            hp = 0;
            aggr = 0;
            isDead = true;
        }
        now.sum--;
        next.sum++;
        loyalty -= K;
    }
};

class Wolf :public Warrior {
public:
    Wolf(int m, int n, int c) :Warrior(m, n, aggrs[4], 4, c) {
        if (color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red wolf " << num << " born" << endl;
        else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue wolf " << num << " born" << endl;
    }

    void beforeWar(Warrior* w) override {//战前抢武器
        int l = w->weapons.size();
        if (w->kind == 4 || l == 0 || weapons.size() >= 10) return;

        sort(w->weapons.begin(), w->weapons.end(), compareAfterfight);//从小到大缴获
        int sum = 0;
        int numGet = w->weapons[0]->kind;//要缴获的武器的编号
        for (auto it = w->weapons.begin(); it != w->weapons.end();) {
            if (weapons.size() >= 10) break;
            if ((*it)->kind == numGet) {
                (*it)->refreshAggr(aggr);//武器重置攻击力
                sum++;
                weapons.push_back((*it));
                it = w->weapons.erase(it);
            }
            else break;
        }

        if (color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red wolf " << num << " took " << sum << ' ' << Weapons[numGet] << " from blue " << Warriors[w->kind] << ' ' << w->num << " in city " << location << endl;
        else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue wolf " << num << " took " << sum << ' ' << Weapons[numGet] << " from red " << Warriors[w->kind] << ' ' << w->num << " in city " << location << endl;
    }
};

void Fight(Warrior* frtW, Warrior* scdW, City& city) {//战斗
    frtW->isNewWar = true;
    scdW->isNewWar = true;
    bool fstop = false;//frtW是否攻击力太低
    bool sstop = false;
    int FlastHp = frtW->hp;//上次受攻击前的血量
    int SlastHp = scdW->hp;
    int FlastW = frtW->weapons.size();//上次攻击前的武器数量
    int SlastW = scdW->weapons.size();
    while(1) {
        //cout << Warriors[scdW->kind] << ' ' << scdW->num << " is being attacked" << endl;
        if (!frtW->isDead) {//先手进攻
            scdW->getHurt(int(frtW->useWeapon()));
            if (scdW->hp == SlastHp) {
                sstop = true;
            }
            else SlastHp = scdW->hp;
        }
        if (!scdW->isDead) {//后手没死就后手进攻
            frtW->getHurt(int(scdW->useWeapon()));
            if (frtW->hp == FlastHp) {
                fstop = true;
            }
            else FlastHp = frtW->hp;
        }


        if (frtW->isDead && scdW->isDead) {//双方均战死
            cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " both red " << Warriors[frtW->color ? frtW->kind : scdW->kind] << ' ' << int(frtW->color ? frtW->num : scdW->num) << " and blue " << Warriors[frtW->color ? scdW->kind : frtW->kind] << ' ' << int(frtW->color ? scdW->num : frtW->num) << " died in city " << city.num << endl;
            frtW->hp = 0;
            frtW->aggr = 0;
            frtW->isDead = true;
            scdW->hp = 0;
            scdW->aggr = 0;
            scdW->isDead = true;
            city.redW = nullptr;
            city.blueW = nullptr;
            city.sum = 0;
            return;
        }

        if (scdW->isDead) {//后手死了
            city.redW = (scdW->color ? nullptr : frtW);
            city.blueW = (!scdW->color ? nullptr : frtW);
            city.sum = 1;
            if (frtW->color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red " << Warriors[frtW->kind] << ' ' << frtW->num << " killed blue " << Warriors[scdW->kind] << ' ' << scdW->num << " in city " << city.num << " remaining " << frtW->hp << " elements" << endl;
            else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue " << Warriors[frtW->kind] << ' ' << frtW->num << " killed red " << Warriors[scdW->kind] << ' ' << scdW->num << " in city " << city.num << " remaining " << frtW->hp << " elements" << endl;
            frtW->getWeapon(scdW);
            scdW->hp = 0;
            scdW->aggr = 0;
            scdW->isDead = true;
            return;
        }

        //cout << Warriors[scdW->kind] << ' ' << scdW->num << " is being attacked" << endl;
        
        if (frtW->isDead) {//先手死了
            city.redW = (frtW->color ? nullptr : scdW);
            city.blueW = (!frtW->color ? nullptr : scdW);
            city.sum = 1;
            if (scdW->color) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " red " << Warriors[scdW->kind] << ' ' << scdW->num << " killed blue " << Warriors[frtW->kind] << ' ' << frtW->num << " in city " << city.num << " remaining " << scdW->hp << " elements" << endl;
            else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " blue " << Warriors[scdW->kind] << ' ' << scdW->num << " killed red " << Warriors[frtW->kind] << ' ' << frtW->num << " in city " << city.num << " remaining " << scdW->hp << " elements" << endl;
            scdW->getWeapon(frtW);
            frtW->hp = 0;
            frtW->aggr = 0;
            frtW->isDead = true;
            return;
        }

        if (((fstop && sstop) || (frtW->weapons.size() == 0 && scdW->weapons.size() == 0) || (frtW->aggr == 0 && scdW->aggr == 0)) && (!frtW->isDead && !scdW->isDead)) {//双方都没死
            if (fstop && sstop) {
                for (int k = 0;k < frtW->weapons.size();k++) {
                    if (frtW->weapons[k]->kind != 0) {
                        Weapon* wp = frtW->weapons[k];
                        frtW->weapons.erase(frtW->weapons.begin() + k);
                        delete wp;
                    }
                }
                for (int k = 0;k < scdW->weapons.size();k++) {
                    if (scdW->weapons[k]->kind != 0) {
                        Weapon* wp = scdW->weapons[k];
                        scdW->weapons.erase(scdW->weapons.begin() + k);
                        delete wp;
                    }
                }
            }
            cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << " both red " << Warriors[frtW->color ? frtW->kind : scdW->kind] << ' ' << int(frtW->color ? frtW->num : scdW->num) << " and blue " << Warriors[frtW->color ? scdW->kind : frtW->kind] << ' ' << int(frtW->color ? scdW->num : frtW->num) << " were alive in city " << city.num << endl;
            return;
        }

    }
}

class HeadQuarter :public City {
public:
    bool rorb;//true为红 flase为蓝
    int yuan;//生命元
    bool cannot = false;//是否还能继续生产
    int number = 1;//统计编号 也是统计已生产了几个武士
    int turn = 0;//记录该生产哪个武士
    bool isConquered = false;//是否被占领了
    int fre[5] = {};
    vector<Warrior*> warriors;

    HeadQuarter(int m, bool x) :yuan(m), rorb(x) {
        if (x) {
            int f[5] = { 2,3,4,1,0 };
            num = 0;
            copy(f, f + 5, fre);
        }
        else {
            int f[5] = { 3,0,1,2,4 };
            num = N + 1;
            copy(f, f + 5, fre);
        }
    }

    void produce() {//生产武士
        if (cannot) return;

        int kind = fre[turn % 5];
        if (yuan < hps[kind]) {
            cannot = true;
            return;
        }

        yuan -= hps[kind];
        if (kind == 0) {
            int c = rorb ? 1 : 0;
            Warrior* dragon = new Dragon(hps[0], number, c);
            dragon->color = rorb ? 1 : 0;
            redW = rorb ? dragon : redW;
            blueW = rorb ? blueW : dragon;
            warriors.push_back(dragon);
        }
        else if (kind == 1) {
            int c = rorb ? 1 : 0;
            Warrior* ninja = new Ninja(hps[1], number, c);
            ninja->color = rorb ? 1 : 0;
            redW = rorb ? ninja : redW;
            blueW = rorb ? blueW : ninja;
            warriors.push_back(ninja);
        }
        else if (kind == 2) {
            int c = rorb ? 1 : 0;
            Warrior* iceman = new Iceman(hps[2], number, c);
            iceman->color = rorb ? 1 : 0;
            redW = rorb ? iceman : redW;
            blueW = rorb ? blueW : iceman;
            warriors.push_back(iceman);
        }
        else if (kind == 3) {
            int c = rorb ? 1 : 0;
            Warrior* lion = new Lion(hps[3], number, c, yuan);
            lion->color = rorb ? 1 : 0;
            redW = rorb ? lion : redW;
            blueW = rorb ? blueW : lion;
            warriors.push_back(lion);
        }
        else if (kind == 4) {
            int c = rorb ? 1 : 0;
            Warrior* wolf = new Wolf(hps[4], number, c);
            wolf->color = rorb ? 1 : 0;
            redW = rorb ? wolf : redW;
            blueW = rorb ? blueW : wolf;
            warriors.push_back(wolf);
        }
        else {
            cout << "Sorry,no such warrior" << endl;
        }
        sum++;
        number++;
        turn++;
        return;
    }

    void reportM() {
        if (rorb) cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << ' ' << yuan << " elements in red headquarter" << endl;
        else cout << setw(3) << setfill('0') << hour << ':' << setw(2) << setfill('0') << minute << ' ' << yuan << " elements in blue headquarter" << endl;
    }

    ~HeadQuarter() {
        for (int i = 0;i < warriors.size();i++) {
            delete warriors[i];
        }
    }
};

void clearIncity(City& city) {//清理城市的尸体
    if (city.redW) {
        if (city.redW->isDead || city.redW->hp <= 0) {
            city.sum--;
            Warrior* wp = city.redW;
            city.redW = nullptr;
            delete wp;
        }
    }
    if (city.blueW) {
        if (city.blueW->isDead || city.blueW->hp <= 0) {
            city.sum--;
            Warrior* wp = city.blueW;
            city.blueW = nullptr;
            delete wp;
        }
    }
}

void clearDead() {//一键清理
    for (int i = 0;i <= N;i++) {
        clearIncity(cities[i]);
    }
}

int main() {
    int t;
    cin >> t;
    for (int i = 1;i <= t;i++) {
        hour = 0;
        minute = 0;
        Rpd = false;
        End = false;
        int M;
        int T;
        cin >> M >> N >> K >> T;
        cin >> hps[0] >> hps[1] >> hps[2] >> hps[3] >> hps[4];
        cin >> aggrs[0] >> aggrs[1] >> aggrs[2] >> aggrs[3] >> aggrs[4];
        cout << "Case " << i <<":"<< endl;
        HeadQuarter Red(M, 1);//创建红司令部
        HeadQuarter Blue(M, 0);//创建蓝
        for (int z = 1;z <= N;z++) {//城市编号
            cities[z].num = z;
            cities[z].sum = 0;
            
            cities[z].redW = nullptr;        
            cities[z].blueW = nullptr;
               
            
        }
        while ((minute + hour * 60) <= T) {
            if (minute == 0) {//司令部生产
                //cout << "producing..." << endl;

                Red.produce();
                Blue.produce();

                //cout << "Done" << endl;
            }
            if (minute == 5) {//lion逃跑
                //cout << "Lion running..." << endl;

                for (int k = 1;k <= N;k++) {
                    if (cities[k].redW) cities[k].redW->Runaway(cities[k]);
                    if (cities[k].blueW) cities[k].blueW->Runaway(cities[k]);
                }

                clearIncity(Red);
                clearDead();
                clearIncity(Blue);

                //cout << "Done" << endl;
            }
            if (minute == 10) {//先前进后汇报
                //cout << "All forwarding..." << endl;
                //蓝武士从左到右
                for (int k = 1; k <= N; ++k) {
                    if (cities[k].blueW) {
                        if (k == 1) cities[k].blueW->Forward(cities[k], Red);
                        else cities[k].blueW->Forward(cities[k], cities[k - 1]);
                    }
                }

                //红武士从右到左
                for (int k = N; k >= 1; --k) {
                    if (cities[k].redW) {
                        if (k == N) cities[k].redW->Forward(cities[k], Blue);
                        else cities[k].redW->Forward(cities[k], cities[k + 1]);
                    }
                }

                //红司令部和蓝司令部的武士移动
                if (Red.redW) {
                    Red.redW->Forward(Red, cities[1]);
                }
                if (Blue.blueW) {
                    Blue.blueW->Forward(Blue, cities[N]);
                }

                // 汇报位置
                // 红司令部
                if (Red.blueW) Red.blueW->tellWhere(Red);
                // 各城市
                for (int k = 1; k <= N; ++k) {
                    if (cities[k].redW) cities[k].redW->tellWhere(cities[k]);
                    if (cities[k].blueW) cities[k].blueW->tellWhere(cities[k]);
                }
                // 蓝司令部
                if (Blue.redW) Blue.redW->tellWhere(Blue);
            }
            if (End) break;
            if (minute == 35) {//wolf战前抢武器
                //cout << "Wolf rubbing..." << endl;

                for (int k = 1;k <= N;k++) {
                    if (cities[k].redW && cities[k].blueW) {
                        cities[k].redW->beforeWar(cities[k].blueW);
                        cities[k].blueW->beforeWar(cities[k].redW);
                    }
                }

                //cout << "Done" << endl;
            }
            if (minute == 40) {//战斗
                //cout << "Fighting..." << endl;

                for (int k = 1;k <= N;k++) {
                    if (cities[k].redW && cities[k].blueW) {
                        if (k % 2 == 1) {
                            Fight(cities[k].redW, cities[k].blueW, cities[k]);
                        }
                        else {
                            Fight(cities[k].blueW, cities[k].redW, cities[k]);
                        }
                        if (cities[k].redW) {
                            cities[k].redW->yell(cities[k]);
                        }
                        if (cities[k].blueW) {
                            cities[k].blueW->yell(cities[k]);
                        }
                    }
                }

                clearIncity(Red);
                clearDead();
                clearIncity(Blue);

                //cout << "Done" << endl;
            }
            if (minute == 50) {//司令部汇报
                //cout << "HQ reporting" << endl;
                Red.reportM();
                Blue.reportM();
                //cout << "Done" << endl;
            }
            if (minute == 55) {//武士汇报
                //cout << "All warriors reporting" << endl;
                if (Red.redW) {
                    Red.redW->reportSelf();
                }
                for (int k = 1;k <= N;k++) {
                    if (cities[k].redW) cities[k].redW->reportSelf();
                    if (cities[k].blueW) cities[k].blueW->reportSelf();
                }
                if (Blue.blueW) {
                    Blue.blueW->reportSelf();
                }
                //cout << "Done" << endl;
            }
            minute += ((minute != 10) ? 5 : 25);
            if (minute >= 60) {
                minute %= 60;
                hour++;
            }
        }
    }
    system("pause");
    return 0;
}
