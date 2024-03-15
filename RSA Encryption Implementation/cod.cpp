#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <set>
#include <numeric>
#include <random>
#include <cmath>

using namespace std;

//in fiecare zi avem autobuze catre urmatoarele orase
vector<string> oraseRomania = {"Bucuresti", "Cluj-Napoca", "Timisoara", "Iasi", "Constanta", "Brasov", "Craiova", "Galati", "Ploiesti", "Oradea"};

class RSA
{
private:
    set<int> prime;  //stocare numere prime
    int public_key;
    int private_key;
    int n; //produsul

    void primefiller();  // umplerea setului 'prime' cu numere prime
    int pickrandomprime(mt19937 &engine);  // selectare numar prim
    void setkeys(mt19937 &engine);  // generare chei
    long long int encrypt(double message);  //criptare

public:
    RSA();  //constructor
    ~RSA() = default;  //destructor 

    vector<int> encoder(string message);  //criptare sir de caractere
    string encryptPassword(const string &password);  //criptare parola
};

RSA::RSA()
{
    primefiller();
    mt19937 engine(42);  //generare numere aleatoare
    setkeys(engine);   //generare chei 
}

void RSA::primefiller()
{
    vector<bool> sieve(250, true);  // Criba Eratosthenes pentru identificarea numerelor prime
    sieve[0] = false;
    sieve[1] = false;

    for (int i = 2; i < 250; i++)
    {
        for (int j = i * 2; j < 250; j += i)
        {
            sieve[j] = false;
        }
    }

    for (int i = 0; i < sieve.size(); i++)
    {
        if (sieve[i])
            prime.insert(i);
    }
}

int RSA::pickrandomprime(mt19937 &engine)
{
    uniform_int_distribution<int> dist(0, prime.size() - 1);   // Distribuție uniformă pentru a alege un indice aleator
    int k = dist(engine);
    auto it = prime.begin();
    advance(it, k);
    int ret = *it;
    prime.erase(it);
    return ret;
}

void RSA::setkeys(mt19937 &engine)
{
    // Se aleg două numere prime aleator
    int prime1 = pickrandomprime(engine);
    int prime2 = pickrandomprime(engine);

    n = prime1 * prime2;
    int fi = (prime1 - 1) * (prime2 - 1);  // Funcția totient Euler
    int e = 2;

    while (1)
    {
        if (std::__gcd(e, fi) == 1)
            break;
        e++;
    }

    public_key = e;
    int d = 2;

    while (1)
    {
        if ((d * e) % fi == 1)
            break;
        d++;
    }

    private_key = d;
}

long long int RSA::encrypt(double message)
{
    int e = public_key;
    long long int encrypted_text = 1;

    while (e--)
    {
        encrypted_text *= message;
        encrypted_text %= n;
    }

    return encrypted_text;
}

vector<int> RSA::encoder(string message)
{
    vector<int> form;

    for (auto &letter : message)
        form.push_back(encrypt(static_cast<int>(letter)));

    return form;
}

string RSA::encryptPassword(const string &password)
{
    vector<int> encrypted = encoder(password);
    stringstream ss;
    for (const auto &num : encrypted)
    {
        ss << num << ' ';
    }
    return ss.str();
}

class CsvAdapter_users
{
protected:
    string fileName;
    vector<string> usernames, passwords;
    RSA rsa; // Adaugam obiectul RSA în clasa CsvAdapter_users

public:
    void read()
    {
        ifstream file(fileName); 

        if (!file.is_open()) {
            cerr << "Eroare la deschidere" << fileName << endl;  //arunca eroare daca fisierul nu poate fi deschis
            exit(1);
        }
        else {
            cout << "Fisierul s-a deschis" << endl;
        }

        //stergem datele din vectori deoarece functia va fi apelata de mai multe ori in cod

        usernames.clear();
        passwords.clear();

        string line;
        getline(file, line); // Sarim peste antet
        
        while (getline(file, line)) {
            istringstream iss(line);
            string username, password;

            getline(iss, username, ';');
            getline(iss, password, ';');

            usernames.push_back(username);
            passwords.push_back(password);
        }

        file.close();  
    }

    void creare_cont(const string &username_nou, const string &parola_noua)
    {
        try
        {
            read();

            if (parola_noua.length() < 6) {
                throw 10;
            }
            string parolaCriptata = rsa.encryptPassword(parola_noua);

            auto gasit_username = find(usernames.begin(), usernames.end(), username_nou);

            if (gasit_username != usernames.end())
            {
                throw 10.1;
            }

            ofstream file(fileName, ios::app);

            if (!file.is_open())
            {
                cerr << "Eroare la deschidere" << fileName << endl;
                exit(1);
            }

            file << username_nou << ";" << parolaCriptata << "\n";
            cout << "Contul a fost creat cu succes. Bine ati venit pe site-ul nostru!" << endl;

            usernames.push_back(username_nou);
            passwords.push_back(parolaCriptata);

            file.close();
        }
        catch (int exceptie)
        {
            cout << "Contul nu s-a putut crea, introduceti o noua parola" << endl;

            string parola_utilizator_nou;
            cout << "Introduceti o alta parola : ";
            cin >> parola_utilizator_nou;

            creare_cont(username_nou, parola_utilizator_nou);
        }
        catch (double exceptie)
        {
            cout << "Contul nu s-a putut crea, introduceti un alt username" << endl;
            string utilizator_nou_1;

            cout << "Introduceti un alt username : ";
            cin >> utilizator_nou_1;

            creare_cont(utilizator_nou_1, parola_noua);
        }
    }

    CsvAdapter_users(string fileName) : rsa() // Adaugam initializarea obiectului RSA
    {
        this->fileName = fileName;
    }

    CsvAdapter_users() : rsa() {} // Adaugam initializarea obiectului RSA
};

//avem 2 fisiere, unul care contine datele utilizatorilor si altul in care avem excursiile acestora

class CsvAdapter_calatorii {

    //aceasta clasa va fi mostenita

protected:
    string fileName;
    vector<int> zi, luna, an;
    vector<string> destinatie;

public:
    void citire() {
        ifstream file(fileName);

        if (!file.is_open()) {
            cerr << "Eroare la deschiderea fisierului: " << fileName << endl;
            exit(1);
        } else {
            cout << "Fisier deschis cu succes" << endl;
        }

        zi.clear();
        luna.clear();
        an.clear();
        destinatie.clear();

        string line;
        getline(file, line); // Sari peste antet

        while (getline(file, line)) {
            istringstream iss(line);
            string token;

            getline(iss, token, ';');
            zi.push_back(stoi(token));

            getline(iss, token, ';');
            luna.push_back(stoi(token));

            getline(iss, token, ';');
            an.push_back(stoi(token));

            getline(iss, token, ';');
            destinatie.push_back(token);
        }

        file.close();
    }

    void adaugare_cursa(int ziua, int luna, int anul, const string& oras) {
        try {

        // Verificare validitate oras

        auto gasit_oras = find(oraseRomania.begin(), oraseRomania.end(), oras);

        if (gasit_oras == oraseRomania.end()) {
            throw 10;
        }

        // Verificare validitate data

        time_t now = time(0);
        tm* localTime = localtime(&now);
        int zi_curenta = localTime->tm_mday;
        int luna_curenta = localTime->tm_mon + 1; 
        int an_curent = localTime->tm_year + 1900;

        if (anul < an_curent || (anul == an_curent && luna < luna_curenta) || (anul == an_curent && luna == luna_curenta && ziua < zi_curenta) || luna > 12 || ziua > 31) {
            throw 10.1;
        }

        ofstream file(fileName, ios::app);

        if (!file.is_open()) {
            cerr << "Eroare la deschiderea fisierului pentru scriere: " << fileName << endl;
            exit(1);
        } else {
            cout << "Fisier deschis cu succes pentru scriere" << endl;
        }

        // Scriem datele noi la sfarsitul fisierului

        file << ziua << ';' << luna << ';' << anul << ';' << oras << '\n';

        file.close();

        // Afișează informatiile despre cursa

        cout << "Cursa adaugata pentru data de " << ziua << "/" << luna << "/" << anul << " catre " << oras << endl;
        }catch (int exceptie) {
            cout << "Introdu un oras valid:" << endl;
            string oras_nou;
            cin >> oras_nou;
            adaugare_cursa(ziua, luna, anul, oras_nou);     
        }

        catch (double exceptie) {
            cout << "Data introdusa este gresita. Introduceti o data valida." << endl;
            int ziua1, luna1, anul1;

            cout << "Ziua :";
            cin >> ziua1;

            cout << "Luna :";
            cin >> luna1;

            cout << "Anul :";
            cin >> anul1;

            adaugare_cursa(ziua1, luna1, anul1, oras);
  
        }
    }

    void stergere_cursa(int ziua, int luna, int anul, const string& oras) {
    try {
        int ok = 0; //variabila care ne arata daca s-a gasit vreo cursa
        citire();

        // Creati un nou vector si il umpleti doar cu elementele pe care nu le doriti șterse

        vector<int> noile_zi;
        vector<int> noile_luna;
        vector<int> noile_an;
        vector<string> noile_destinatii;

        //gestionam cazul in care dorim sa stergem o calatorie dar deja le am sters pe toate
        if (zi.size() == 0) {
            throw 10;
        }

        for (size_t i = 0; i < zi.size(); ++i) {
            if (!(zi[i] == ziua && luna == luna && an[i] == anul && destinatie[i] == oras)) {
                noile_zi.push_back(zi[i]);
                noile_luna.push_back(luna);
                noile_an.push_back(an[i]);
                noile_destinatii.push_back(destinatie[i]);
            }
            else {
                ok = 1;
            }
        }

        // Suprascriem vectorul original cu noul vector fara cursele sterse

        zi = noile_zi;
        this->luna = noile_luna;
        an = noile_an;
        destinatie = noile_destinatii;

        // Rescriem fisierul
        rescrie_fisier();

        if (ok == 0) {
            throw "Excursia nu exista!";  
        }
        cout << "Cursa catre " << oras << " pentru data de " << ziua << "/" << luna << "/" << anul << " a fost sterse cu succes." << endl;
    } catch (const char* mesaj) {
        cout << mesaj;
    }

    catch (int x) {
        cout << "Toate calatoriile sunt sterse";
    }
}

void rescrie_fisier() {
    ofstream file(fileName);

    if (!file.is_open()) {
        cerr << "Eroare la deschiderea fisierului pentru rescriere: " << fileName << endl;
        exit(1);
    } else {
        cout << "Fisier deschis cu succes pentru rescriere" << endl;
    }

    // Scriem antetul
    file << "Zi;Luna;An;Destinatie\n";

    // Scriem datele
    for (size_t i = 0; i < zi.size(); ++i) {
        file << zi[i] << ';' << this->luna[i] << ';' << an[i] << ';' << destinatie[i] << '\n';
    }

    file.close();
}
    
    CsvAdapter_calatorii(string fileName) {
        this->fileName = fileName;
    }

    CsvAdapter_calatorii() {}
};

class Utilizator : public CsvAdapter_users{
public:
    Utilizator(string fileName) : CsvAdapter_users(fileName) {}  // Adauga un constructor care inițializează fileName

    void verificaEmail(const string& adresa_email) {
    
    try {
    size_t pozitie = adresa_email.find("@yahoo.com");

    if (pozitie == string::npos || adresa_email.size() > pozitie + 10 || adresa_email == "@yahoo.com") {
        throw 10;
    } 

    cout << "Adresa de email este valida." << endl;

    }catch(int x) {
        cout << "Adresa de mail nu este valida, introdu alta :" << endl;

        string noua_adresa_email;
        cin >> noua_adresa_email;

        verificaEmail(noua_adresa_email);

                    }
    }

    void login(const string& nume, const string& parola) {
    try {
    read();

    string parolacriptata = rsa.encryptPassword(parola);

    auto gasit_username = find(usernames.begin(), usernames.end(), nume);

    if (gasit_username == usernames.end()) {
        throw 10;
    }
    
        // Gaseste pozitia utilizatorului in vector
    int index = distance(usernames.begin(), gasit_username);

        // Verifică dacă parola corespunzătoare se află pe aceeași poziție
    if (passwords[index] != parolacriptata) {
        throw 10.1;
    }
    cout << "Autentificare reusita, cu ce va putem ajuta azi?" << endl; 


    }catch(...) {
        // Dacă nu a fost găsit utilizatorul sau parola nu corespunde, cere reîncercarea
    cout << "Nume sau parola gresita. Incearca din nou." << endl;

    string nume1;
    string parola1;

    cout << "Rescrieti numele : " << endl;
    cin >> nume1;

    cout << "Rescrieti parola : " << endl;
    cin >> parola1;

    login(nume1, parola1);
    }
}
};

class Cursa : public CsvAdapter_calatorii{
public:
    Cursa(string fileName) : CsvAdapter_calatorii(fileName) {}  // Adaugă un constructor care inițializează fileName
  
};

int main() {
    int alegere, actiune;
    string email, username, password;
    int zi, luna, an;
    string oras;
    Utilizator user("conturi.csv");
    Cursa autobuz("excursii.csv");
    
    cout << "1. Log in\n";
    cout << "2. Creaza cont\n";
    cout << "Apasati orice alta tasta pentru a parasi site-ul\n";
    cout << "Enter your choice: ";
    cin >> alegere;


    switch (alegere) {
        case 1:
            cout << "Introdu email: ";
            cin >> email;
            user.verificaEmail(email);

            cout << "Introd username: ";
            cin >> username;

            cout << "Introd parola: ";
            cin >> password;

            user.login(username, password);
            break;

        case 2:
            cout << "Introdu email: ";
            cin >> email;
            user.verificaEmail(email);

            cout << "Introd username: ";
            cin >> username;

            cout << "Introdu parola: ";
            cin >> password;

            user.creare_cont(username, password);
            break;

        default:
            cout << "Va multumim ca ne ati folosit site-ul!\n";
            return 1;
    }

    do {
        cout << "Ce doriti sa faceti?\n";
        cout << "1. Adaugare cursa\n";
        cout << "2. Stergere cursa\n";
        cout << "Apasati orice alta tasta pentru a parasi siteul\n";
        cin >> actiune;

        switch (actiune) {
            case 1:
                cout << "Adaugati datele cursei:\n";
                cout << "Ziua : ";
                cin >> zi;
                cout << "Luna : ";
                cin >> luna;
                cout << "Anul : ";
                cin >> an;
                cout << "Orasul : ";
                cin >> oras;
                autobuz.adaugare_cursa(zi, luna, an, oras);
                break;

            case 2:
                cout << "Stergeti datele cursei:\n";
                cout << "Ziua : ";
                cin >> zi;
                cout << "Luna : ";
                cin >> luna;
                cout << "Anul : ";
                cin >> an;
                cout << "Orasul : ";
                cin >> oras;
                autobuz.stergere_cursa(zi, luna, an, oras);
                break;

            default:
                cout << "Va multumim ca ne ati folosit site-ul!\n";
                return 0;
        }

    } while (true);
    
    return 0;
}
