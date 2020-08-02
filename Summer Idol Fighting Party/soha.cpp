#include <bits/stdc++.h>
using namespace std;

string Log;
bool RecordLog = 0;
void Record(const char* str) { if (RecordLog) Log += string(str) + " "; }
void Record(string str) { if (RecordLog) Log += str + " "; }
template <class Damage, class R>
void Record(Damage damage, R rival) {
    if (!RecordLog) return;
    damage.val -= damage.elem ? 0 : rival.DEF;
    damage.val = max(0, damage.val);
    Log += damage.Info() + " --> " + rival.ID + "\n";
}
template <class R1, class R2>
void Record(R1 A, R2 B, int f) {
    if (!RecordLog) return;
    if (f) Log += A.Info() + " " + B.Info() + "\n";
    else Log += B.Info() + " " + A.Info() + "\n";
}

//random_device rd;
mt19937 rng(time(NULL));
struct Random_Number_Generator {
    int range(int l, int r) { return uniform_int_distribution <int> (l, r) (rng); }
    bool percent(int x) { return range(1, 100) <= x; }
} rnd;

struct Damage {
    int elem, ult, val, Diz, Sil;
    Damage () : elem(0), ult(0), val(0), Diz(0), Sil(0) {}
    Damage (int _elem, int _ult, int _val, int _Diz = 0, int _Sil = 0)
        : elem(_elem), ult(_ult), val(_val), Diz(_Diz), Sil(_Sil) {}
    string Info() {
        return "Damage[" + to_string(elem) + ", " + to_string(ult) + ", "
                + to_string(val) + ", " + to_string(Diz) + ", " + to_string(Sil) + "]";
    }
};

struct Role {
    string ID;
    int HP, ATK, DEF, SPE, CNT, ACC;
    int Diz, Sil;
    
    Role (string _ID, int _HP, int _ATK, int _DEF, int _SPE, int _CNT)
        : ID(_ID), HP(_HP), ATK(_ATK), DEF(_DEF), SPE(_SPE), CNT(_CNT), ACC(100), Diz(0), Sil(0) {}

    string Info() { return ID + "[" + to_string(HP) + "]"; }
    //string Info() { return ID + "[" + to_string(HP) + " " + to_string(ATK) + " " + to_string(ACC) + "]"; }

    bool alive() { return HP > 0; }
    bool dizzy() { return Diz ? Diz-- : 0; }
    bool silence() { return Sil ? Sil-- : 0; }
    bool miss() { return !rnd.percent(ACC); }

    Damage attack() { return Damage(0, 0, ATK); }
    Damage defense(Damage damage) {
        HP -= max(0, damage.val - (damage.elem ? 0 : DEF));
        if (damage.Diz) Diz = damage.Diz;
        if (damage.Sil) Sil = damage.Sil;
        return Damage();
    }

    template <class R>
    void apply(Damage damage, R &rival) {
        if (!miss()) {
            Record(damage, rival);
            Damage ret = rival.defense(damage);
            if (ret.val) defense(damage);
        } else {
            Record("miss");
        }
    }
};

struct Game {
    int round;
    string winner;

    Game () { Log = ""; }

    template <class R1, class R2>
    void start(R1 A, R2 B) {
        int AtoB = A.SPE > B.SPE, f = AtoB;
        for (round = 1; A.alive() && B.alive(); round += ((f ^= 1) == AtoB)) {
            Record("Round " + to_string(round) + " :  ");
            Record(A, B, AtoB);
            f ? A.battle(round, B) : B.battle(round, A);
        }
        winner = A.alive() ? A.ID : B.ID;
    }

    void PrintLog() { cout << Log << endl; }
};

struct Kiana : public Role {
    Kiana () : Role("kiana", 100, 24, 11, 23, 1) {}

    bool cooldown(int round) { return round % 2 == 0; }

    void skill_1(Damage &damage, int def) {
        damage.val += 2 * def;
        damage.ult = 1;
    }

    void skill_2() { Diz = rnd.percent(35); }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        Damage damage = attack();
        if (!silence() && cooldown(round)) {
            skill_1(damage, rival.DEF);
            skill_2();
        } else {}

        apply(damage, rival);
    }
} kiana;

struct Mei : public Role {
    Mei () : Role("mei", 100, 22, 12, 30, 1) {}
    
    bool cooldown(int round) { return round % 2 == 0; }

    void skill_1(Damage &damage) { if (rnd.percent(30)) damage.Diz = 1; }

    template <class R>
    void skill_2(R &rival) {
        Damage damage(1, 1, 3);
        skill_1(damage);
        for (int i = 0; i < 5; ++i) apply(damage, rival);
    }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        int tmp = Sil;
        Damage damage;
        if (!silence() && cooldown(round)) {
            skill_2(rival);
        } else {
            damage = attack();
            if (!tmp) skill_1(damage);
            apply(damage, rival);
        }
    }
} mei;

struct Bronya : public Role {
    Bronya () : Role("bronya", 100, 21, 10, 20, 1) {}
    
    bool cooldown(int round) { return round % 3 == 0; }

    template <class R>
    void skill_1(R &rival) {
        if (!rnd.percent(25)) return;
        Damage damage(0, 0, 12);
        for (int i = 0; i < 4; ++i) apply(damage, rival);
    }

    template <class R>
    void skill_2(R &rival) {
        Damage damage(1, 1, rnd.range(1, 100));
        apply(damage, rival);
    }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        Damage damage;
        if (!silence() && cooldown(round)) {
            skill_2(rival);
        } else {
            damage = attack();
            apply(damage, rival);
            skill_1(rival);
        }
    }
} bronya;

struct Seele : public Role {
    int state;
    Seele () : Role("seele", 100, 23, 13, 26, 1), state(0) {}

    void skill_1() { state ^= 1; }

    void skill_2() {
        if (!state) { HP = min(100, HP + rnd.range(1, 15)); ATK -= 10; DEF += 5; }
        else { ATK += 10; DEF -= 5; }
    }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        if (!silence()) {
            skill_1();
            skill_2();
        }
        Damage damage = attack();
        apply(damage, rival);
    }
} seele;

struct Sakura_Kallen : public Role {
    Sakura_Kallen () : Role("sakura_kallen", 100, 20, 9, 18, 2) {}
    
    bool cooldown(int round) { return round % 2 == 0; }

    void skill_1() {
        if (!rnd.percent(30)) return;
        HP = min(100, HP + 25);
    }

    template <class R>
    void skill_2(R &rival) {
        Damage damage(1, 1, 25);
        apply(damage, rival);
    }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        if (!Sil) skill_1();
        if (!silence() && cooldown(round)) {
            skill_2(rival);
        } else {
            //skill_1();
            Damage damage = attack();
            apply(damage, rival);
        }
    }
} sakura_kallen;

struct Rozaliya_Liliya : public Role {
    int buff, relive;
    Rozaliya_Liliya () : Role("rozaliya_liliya", 100, 18, 10, 10, 2), buff(0), relive(0) {}

    void skill_1() {
        HP = 20;
        relive = buff = 1;
    }

    template <class R>
    void skill_2(R &rival) {
        Damage damage;
        if (rnd.percent(50)) {
            damage = Damage(0, 1, 233);
        } else {
            damage = Damage(0, 1, 50);
        }
        apply(damage, rival);
    }

    Damage defense(Damage damage) {
        HP -= max(0, damage.val - (damage.elem ? 0 : DEF));
        if (damage.Diz) Diz = damage.Diz;
        if (damage.Sil) Sil = damage.Sil;
        if (!alive() && !relive && !Sil) skill_1();
        return Damage();
    }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        if (!silence() && buff) {
            skill_2(rival);
            buff = 0;
        } else {
            buff = 0;
            Damage damage = attack();
            apply(damage, rival);
        }
    }
} rozaliya_liliya;

struct Durandal : public Role {
    Durandal () : Role("durandal", 100, 19, 10, 15, 1) {}

    void skill_1() { ATK += 3; }

    Damage skill_2() { return Damage(0, 0, 30); }

    Damage defense(Damage damage) {
        if (damage.ult == 1 && rnd.percent(16)) {
            return skill_2();
        } else {
            HP -= max(0, damage.val - (damage.elem ? 0 : DEF));
            if (damage.Diz) Diz = damage.Diz;
            if (damage.Sil) Sil = damage.Sil;
        }
        return Damage();
    }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        if (!silence()) skill_1();
        
        Damage damage = attack();
        apply(damage, rival);
    }
} durandal;

struct FuHua : public Role {
    FuHua () : Role("fuhua", 100, 17, 15, 16, 1) {}
    
    bool cooldown(int round) { return round % 3 == 0; }

    void skill_1(Damage &damage) { damage.elem = 1; }

    template <class R>
    void skill_2(R &rival) {
        Damage damage(1, 1, 18);
        rival.ACC = max(0, rival.ACC - 25);
        apply(damage, rival);
    }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        if (!silence() && cooldown(round)) {
            skill_2(rival);
        } else {
            Damage damage = attack();
            skill_1(damage);
            apply(damage, rival);
        }
    }
} fuhua;

struct Theresa : public Role {
    Theresa () : Role("theresa", 100, 19, 12, 22, 3) {}
    
    bool cooldown(int round) { return round % 3 == 0; }

    template <class R>
    void skill_1(R &rival) { if (!miss() && rnd.percent(30)) rival.DEF -= 5;/*= max(0, rival.DEF - 5);*/ }

    template <class R>
    void skill_2(R &rival) {
        Damage damage(0, 1, 16);
        for (int i = 0; i < 5; ++i) apply(damage, rival);
    }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        int tmp = Sil;
        if (!silence() && cooldown(round)) {
            skill_2(rival);
        } else {
            Damage damage = attack();
            apply(damage, rival);
        }
        if (!tmp) skill_1(rival);
    }
} theresa;

struct Rita : public Role {
    int debuff;
    Rita () : Role("rita", 100, 26, 11, 17, 1), debuff(0) {}
    
    bool cooldown(int round) { return round % 4 == 0; }

    template <class R>
    void skill_1(Damage &damage, R &rival) {
        if (rnd.percent(35)) {
            damage.val = max(0, damage.val - 3);
            rival.ATK = max(0, rival.ATK - 4);
        }
    }

    template <class R>
    void skill_2(R &rival) {
        rival.HP = min(100, rival.HP + 4);
        rival.Sil = 2;
        debuff = 1;
    }

    Damage defense(Damage damage) {
        damage.val = max(0, damage.val - (damage.elem ? 0 : DEF));
        if (debuff) damage.val = (int)round(damage.val * 0.4);
        HP -= damage.val;
        if (damage.Diz) Diz = damage.Diz;
        if (damage.Sil) Sil = damage.Sil;
        return Damage();
    }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        if (!silence() && cooldown(round)) {
            skill_2(rival);
        } else {
            Damage damage = attack();
            skill_1(damage, rival);
            apply(damage, rival);
        }
    }
} rita;

struct Himeko : public Role {
    Himeko () : Role("himeko", 100, 23, 9, 12, 1) {}

    bool cooldown(int round) { return round % 2 == 0; }

    // real_damage = val - DEF
    // real_damage = (ATK - DEF) * 2
    // val = 2*ATK - 2*DEF + DEF = 2*ATK - DEF
    template <class R>
    void skill_1(Damage &damage, R rival) {
        if (rival.CNT > 1 && ATK > rival.DEF) damage.val = 2 * ATK - rival.DEF;
    }

    void skill_2() { ATK *= 2; ACC = max(0, ACC - 35); }

    template <class R>
    void battle(int round, R &rival) {
        if (dizzy()) return;

        if (!silence() && cooldown(round)) {
            skill_2();
        } else {}

        Damage damage = attack();
        skill_1(damage, rival);
        apply(damage, rival);
    }
} himeko;

struct Natasha : public Role {
    int check, buff;
    Natasha () : Role("natasha", 100, 23, 14, 14, 1), check(0), buff(0) {}

    bool cooldown(int round) { return round % 3 == 0; }

    //real_damage = val - DEF
    //real_damage = (ATK - DEF) * 1.25
    //val = 1.25 * ATK - 0.25 * DEF
    template <class R>
    void skill_1(Damage &damage, R rival) {
        if (buff && damage.val > rival.DEF) {
            damage.val = (int)round(1.25 * damage.val - 0.25 * rival.DEF);
        }
    }

    template <class R>
    void skill_2(R &rival) {
        for (int i = 0; i < 7; ++i) {
            Damage damage(0, 1, 16);
            skill_1(damage, rival);
            apply(damage, rival);
        }
    }

    template <class R>
    void battle(int round, R &rival) {
        if (!check) {
            buff = (rival.ID == "kiana" || rnd.percent(25));
            check = 1;
        }
        if (dizzy()) return;

        if (!silence() && cooldown(round)) {
            skill_2(rival);
        } else {
            Damage damage = attack();
            skill_1(damage, rival);
            apply(damage, rival);
        }
    }
} natasha;

// kiana mei bronya seele sakura_kallen rozaliya_liliya durandal fuhua theresa rita himeko natasha

#define Left mei
#define Right rozaliya_liliya

int main()
{
    RecordLog = 0;
    int Count = 10000, A = 0, B = 0;
    for (int i = 0; i < Count; ++i) {
        Game game;
        game.start(Left, Right);
        game.winner == Left.ID ? ++A : ++B;
        if (RecordLog) game.PrintLog();
    }
    printf("\n\n%s: %.2f%% | %s: %.2f%%\n\n", 
        Left.ID.c_str(), (double)A * 100 / Count, Right.ID.c_str(), (double)B * 100 / Count);
    return 0;
}

/*
    string id;
    cout << "\nRole's id list:\n";
    cout << "kiana mei bronya seele sakura_kallen rozaliya_liliya\n";
    cout << "durandal fuhua theresa rita himeko natasha\n\n";
    cout << "Please input player_1's id: ";
    cin >> id;
    Role& Left = player(id);
    cout << "Please input player_2's id: ";
    cin >> id;
    Role& Right = player(id);

//template <class R>
Role& player(string id) {
    if (id == "kiana")            return kiana;
    if (id == "mei")              return mei;
    if (id == "bronya")           return bronya;
    if (id == "seele")            return seele;
    if (id == "sakura_kallen")    return sakura_kallen;
    if (id == "rozaliya_liliya")  return rozaliya_liliya;
    if (id == "durandal")         return durandal;
    if (id == "fuhua")            return fuhua;
    if (id == "theresa")          return theresa;
    if (id == "rita")             return rita;
    if (id == "himeko")           return himeko;
    if (id == "natasha")          return natasha;
    return kiana;
}
*/