#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Resultado {
    int base;
    int lado;
    int celulas;

    string puzzle;
    string algoritmo;

    int repeticao;

    double tempo_us;
    double tempo_ms;

    long long recursive_calls;
    long long guesses;
    long long backtracks;
    long long singles;
    long long tuples;

    string status;
};

struct Grupo {
    vector<double> tempo;
    vector<double> recursive;
    vector<double> guesses;
    vector<double> backtracks;
    vector<double> singles;
    vector<double> tuples;
};

vector<string> splitCSV(const string& linha) {
    vector<string> campos;
    string campo;

    stringstream ss(linha);

    while (getline(ss, campo, ',')) {
        campos.push_back(campo);
    }

    return campos;
}

vector<Resultado> lerCSV(const string& nome) {
    ifstream arquivo(nome);

    if (!arquivo.is_open()) {
        cerr << "ERRO: nao foi possivel abrir " << nome << "\n";
        exit(1);
    }

    vector<Resultado> resultados;

    string linha;

    // Ignora o cabecalho
    getline(arquivo, linha);

    while (getline(arquivo, linha)) {

        if (linha.empty())
            continue;

        vector<string> c = splitCSV(linha);

        if (c.size() < 14)
            continue;

        Resultado r;

        r.base = stoi(c[0]);
        r.lado = stoi(c[1]);
        r.celulas = stoi(c[2]);

        r.puzzle = c[3];
        r.algoritmo = c[4];

        r.repeticao = stoi(c[5]);

        r.tempo_us = stod(c[6]);
        r.tempo_ms = stod(c[7]);

        r.recursive_calls = stoll(c[8]);
        r.guesses = stoll(c[9]);
        r.backtracks = stoll(c[10]);
        r.singles = stoll(c[11]);
        r.tuples = stoll(c[12]);

        r.status = c[13];

        resultados.push_back(r);
    }

    return resultados;
}

double media(const vector<double>& valores) {

    if (valores.empty())
        return 0.0;

    double soma = 0.0;

    for (double x : valores)
        soma += x;

    return soma / valores.size();
}

string escaparXML(const string& texto) {

    string resultado;

    for (char c : texto) {

        switch (c) {

            case '&':
                resultado += "&amp;";
                break;

            case '<':
                resultado += "&lt;";
                break;

            case '>':
                resultado += "&gt;";
                break;

            case '"':
                resultado += "&quot;";
                break;

            default:
                resultado += c;
        }
    }

    return resultado;
}

double logSeguro(double x) {

    if (x <= 0)
        return 0.0;

    return log10(x);
}

/*
============================================================
CLASSE PARA GERAR GRAFICOS SVG
============================================================
*/

class Grafico {

private:

    ofstream out;

    int largura;
    int altura;

    int margemEsquerda;
    int margemDireita;
    int margemSuperior;
    int margemInferior;

public:

    Grafico(
        const string& nome,
        const string& titulo,
        const string& eixoX,
        const string& eixoY
    ) {

        largura = 1200;
        altura = 800;

        margemEsquerda = 110;
        margemDireita = 60;
        margemSuperior = 90;
        margemInferior = 100;

        out.open("results/graficos/" + nome);

        out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            << "width=\"" << largura
            << "\" height=\"" << altura << "\">\n";

        out << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";

        out << "<text x=\"" << largura / 2
            << "\" y=\"45\" "
            << "text-anchor=\"middle\" "
            << "font-family=\"Arial\" "
            << "font-size=\"26\" "
            << "font-weight=\"bold\">"
            << escaparXML(titulo)
            << "</text>\n";

        int x0 = margemEsquerda;
        int y0 = altura - margemInferior;

        int x1 = largura - margemDireita;
        int y1 = margemSuperior;

        // Eixo X
        out << "<line x1=\"" << x0
            << "\" y1=\"" << y0
            << "\" x2=\"" << x1
            << "\" y2=\"" << y0
            << "\" stroke=\"black\" stroke-width=\"2\"/>\n";

        // Eixo Y
        out << "<line x1=\"" << x0
            << "\" y1=\"" << y0
            << "\" x2=\"" << x0
            << "\" y2=\"" << y1
            << "\" stroke=\"black\" stroke-width=\"2\"/>\n";

        // Labels
        out << "<text x=\"" << largura / 2
            << "\" y=\"" << altura - 25
            << "\" text-anchor=\"middle\" "
            << "font-family=\"Arial\" font-size=\"18\">"
            << escaparXML(eixoX)
            << "</text>\n";

        out << "<text x=\"25\" y=\"" << altura / 2
            << "\" text-anchor=\"middle\" "
            << "font-family=\"Arial\" font-size=\"18\" "
            << "transform=\"rotate(-90 25 "
            << altura / 2 << ")\">"
            << escaparXML(eixoY)
            << "</text>\n";
    }

    double X(double x, double xmin, double xmax) {

        double larguraGrafico =
            largura - margemEsquerda - margemDireita;

        if (xmax == xmin)
            return margemEsquerda;

        return margemEsquerda +
            (x - xmin) /
            (xmax - xmin) *
            larguraGrafico;
    }

    double Y(double y, double ymin, double ymax) {

        double alturaGrafico =
            altura - margemSuperior - margemInferior;

        if (ymax == ymin)
            return altura - margemInferior;

        return altura - margemInferior -
            (y - ymin) /
            (ymax - ymin) *
            alturaGrafico;
    }

    void grade(
        double xmin,
        double xmax,
        double ymin,
        double ymax
    ) {

        for (int i = 0; i <= 5; i++) {

            double valor =
                ymin +
                (ymax - ymin) * i / 5.0;

            double y = Y(valor, ymin, ymax);

            out << "<line x1=\"" << margemEsquerda
                << "\" y1=\"" << y
                << "\" x2=\"" << largura - margemDireita
                << "\" y2=\"" << y
                << "\" stroke=\"#dddddd\" "
                << "stroke-width=\"1\"/>\n";

            out << "<text x=\""
                << margemEsquerda - 10
                << "\" y=\"" << y + 5
                << "\" text-anchor=\"end\" "
                << "font-family=\"Arial\" "
                << "font-size=\"13\">"
                << fixed << setprecision(2)
                << valor
                << "</text>\n";
        }

        for (int i = 0; i <= 5; i++) {

            double valor =
                xmin +
                (xmax - xmin) * i / 5.0;

            double x = X(valor, xmin, xmax);

            out << "<line x1=\"" << x
                << "\" y1=\"" << margemSuperior
                << "\" x2=\"" << x
                << "\" y2=\""
                << altura - margemInferior
                << "\" stroke=\"#eeeeee\" "
                << "stroke-width=\"1\"/>\n";

            out << "<text x=\"" << x
                << "\" y=\""
                << altura - margemInferior + 25
                << "\" text-anchor=\"middle\" "
                << "font-family=\"Arial\" "
                << "font-size=\"13\">"
                << fixed << setprecision(0)
                << valor
                << "</text>\n";
        }
    }

    void linha(
        const vector<pair<double,double>>& pontos,
        double xmin,
        double xmax,
        double ymin,
        double ymax,
        const string& legenda,
        int tipo
    ) {

        if (pontos.empty())
            return;

        string cor;

        if (tipo == 0)
            cor = "#2563eb";
        else if (tipo == 1)
            cor = "#dc2626";
        else if (tipo == 2)
            cor = "#16a34a";
        else
            cor = "#9333ea";

        string path;

        for (size_t i = 0; i < pontos.size(); i++) {

            double x = X(pontos[i].first, xmin, xmax);
            double y = Y(pontos[i].second, ymin, ymax);

            if (i == 0)
                path += "M ";
            else
                path += " L ";

            path += to_string(x);
            path += " ";
            path += to_string(y);
        }

        out << "<path d=\"" << path
            << "\" fill=\"none\" "
            << "stroke=\"" << cor
            << "\" stroke-width=\"3\"/>\n";

        for (auto p : pontos) {

            double x = X(p.first, xmin, xmax);
            double y = Y(p.second, ymin, ymax);

            out << "<circle cx=\"" << x
                << "\" cy=\"" << y
                << "\" r=\"5\" fill=\""
                << cor << "\"/>\n";
        }

        // legenda
        double lx =
            largura - margemDireita - 190;

        double ly =
            margemSuperior + 25 + tipo * 30;

        out << "<line x1=\"" << lx
            << "\" y1=\"" << ly
            << "\" x2=\"" << lx + 30
            << "\" y2=\"" << ly
            << "\" stroke=\"" << cor
            << "\" stroke-width=\"3\"/>\n";

        out << "<text x=\"" << lx + 40
            << "\" y=\"" << ly + 5
            << "\" font-family=\"Arial\" "
            << "font-size=\"15\">"
            << escaparXML(legenda)
            << "</text>\n";
    }

    void pontos(
        const vector<pair<double,double>>& valores,
        double xmin,
        double xmax,
        double ymin,
        double ymax,
        int tipo
    ) {

        string cor;

        if (tipo == 0)
            cor = "#2563eb";
        else if (tipo == 1)
            cor = "#dc2626";
        else
            cor = "#16a34a";

        for (auto p : valores) {

            double x = X(p.first, xmin, xmax);
            double y = Y(p.second, ymin, ymax);

            out << "<circle cx=\"" << x
                << "\" cy=\"" << y
                << "\" r=\"5\" fill=\""
                << cor << "\"/>\n";
        }
    }

    ~Grafico() {

        if (out.is_open()) {
            out << "</svg>\n";
            out.close();
        }
    }
};

/*
============================================================
1. TEMPO MEDIO
============================================================
*/

void graficoTempo(
    const vector<Resultado>& resultados
) {

    map<pair<string,int>, vector<double>> dados;

    for (const auto& r : resultados) {

        if (r.status != "solved")
            continue;

        dados[{r.algoritmo, r.celulas}]
            .push_back(r.tempo_ms);
    }

    map<string, vector<pair<double,double>>> series;

    for (const auto& [chave, valores] : dados) {

        series[chave.first].push_back({
            static_cast<double>(chave.second),
            media(valores)
        });
    }

    double ymax = 0;

    for (auto& [nome, pontos] : series) {

        sort(
            pontos.begin(),
            pontos.end()
        );

        for (auto p : pontos)
            ymax = max(ymax, p.second);
    }

    ymax *= 1.1;

    if (ymax == 0)
        ymax = 1;

    Grafico g(
        "01_tempo_medio.svg",
        "Tempo medio de resolucao por tamanho da entrada",
        "Numero de celulas (N)",
        "Tempo medio (ms)"
    );

    g.grade(0, 650, 0, ymax);

    if (series.count("backtracking"))
        g.linha(
            series["backtracking"],
            0, 650, 0, ymax,
            "Backtracking",
            0
        );

    if (series.count("rule_based"))
        g.linha(
            series["rule_based"],
            0, 650, 0, ymax,
            "Rule-based",
            1
        );
}

/*
============================================================
2. TEMPO EM ESCALA LOG
============================================================
*/

void graficoTempoLog(
    const vector<Resultado>& resultados
) {

    map<pair<string,int>, vector<double>> dados;

    for (const auto& r : resultados) {

        if (r.status != "solved")
            continue;

        if (r.tempo_ms <= 0)
            continue;

        dados[{r.algoritmo, r.celulas}]
            .push_back(r.tempo_ms);
    }

    map<string, vector<pair<double,double>>> series;

    double ymin = 1e100;
    double ymax = 0;

    for (const auto& [chave, valores] : dados) {

        double t = media(valores);

        if (t <= 0)
            continue;

        double y = log10(t);

        series[chave.first].push_back({
            static_cast<double>(chave.second),
            y
        });

        ymin = min(ymin, y);
        ymax = max(ymax, y);
    }

    ymin -= 0.2;
    ymax += 0.2;

    Grafico g(
        "02_tempo_log.svg",
        "Tempo de execucao em escala logaritmica",
        "Numero de celulas (N)",
        "log10(tempo em ms)"
    );

    g.grade(0, 650, ymin, ymax);

    if (series.count("backtracking"))
        g.linha(
            series["backtracking"],
            0, 650, ymin, ymax,
            "Backtracking",
            0
        );

    if (series.count("rule_based"))
        g.linha(
            series["rule_based"],
            0, 650, ymin, ymax,
            "Rule-based",
            1
        );
}

/*
============================================================
3. SPEEDUP
============================================================
*/

void graficoSpeedup(
    const vector<Resultado>& resultados
) {

    map<int, vector<double>> bt;
    map<int, vector<double>> rb;

    for (const auto& r : resultados) {

        if (r.status != "solved")
            continue;

        if (r.algoritmo == "backtracking")
            bt[r.celulas].push_back(r.tempo_ms);

        if (r.algoritmo == "rule_based")
            rb[r.celulas].push_back(r.tempo_ms);
    }

    vector<pair<double,double>> pontos;

    double ymax = 0;

    for (auto [n, valores] : bt) {

        if (!rb.count(n))
            continue;

        double tbt = media(valores);
        double trb = media(rb[n]);

        if (trb <= 0)
            continue;

        double speedup = tbt / trb;

        pontos.push_back({
            static_cast<double>(n),
            speedup
        });

        ymax = max(ymax, speedup);
    }

    ymax *= 1.1;

    if (ymax == 0)
        ymax = 1;

    Grafico g(
        "03_speedup.svg",
        "Speedup: Backtracking / Rule-based",
        "Numero de celulas (N)",
        "Speedup"
    );

    g.grade(0, 650, 0, ymax);

    g.linha(
        pontos,
        0, 650, 0, ymax,
        "Speedup",
        0
    );
}

/*
============================================================
4. CHAMADAS RECURSIVAS
============================================================
*/

void graficoMetricas(
    const vector<Resultado>& resultados,
    const string& metrica,
    const string& arquivo,
    const string& titulo,
    const string& eixoY
) {

    map<int, vector<double>> dados;

    for (const auto& r : resultados) {

        if (r.algoritmo != "backtracking")
            continue;

        if (r.status != "solved")
            continue;

        double valor = 0;

        if (metrica == "recursive")
            valor = r.recursive_calls;

        else if (metrica == "guesses")
            valor = r.guesses;

        else if (metrica == "backtracks")
            valor = r.backtracks;

        dados[r.celulas].push_back(valor);
    }

    vector<pair<double,double>> pontos;

    double ymax = 0;

    for (const auto& [n, valores] : dados) {

        double m = media(valores);

        pontos.push_back({
            static_cast<double>(n),
            m
        });

        ymax = max(ymax, m);
    }

    ymax *= 1.1;

    if (ymax == 0)
        ymax = 1;

    Grafico g(
        arquivo,
        titulo,
        "Numero de celulas (N)",
        eixoY
    );

    g.grade(0, 650, 0, ymax);

    g.linha(
        pontos,
        0, 650, 0, ymax,
        "Backtracking",
        0
    );
}

/*
============================================================
5. SINGLES / TUPLES
============================================================
*/

void graficoRuleBased(
    const vector<Resultado>& resultados
) {

    map<int, vector<double>> singles;
    map<int, vector<double>> tuples;

    for (const auto& r : resultados) {

        if (r.algoritmo != "rule_based")
            continue;

        if (r.status != "solved")
            continue;

        singles[r.celulas].push_back(r.singles);
        tuples[r.celulas].push_back(r.tuples);
    }

    vector<pair<double,double>> s;
    vector<pair<double,double>> t;

    double ymax = 0;

    for (auto [n, valores] : singles) {

        double m = media(valores);

        s.push_back({
            static_cast<double>(n),
            m
        });

        ymax = max(ymax, m);
    }

    for (auto [n, valores] : tuples) {

        double m = media(valores);

        t.push_back({
            static_cast<double>(n),
            m
        });

        ymax = max(ymax, m);
    }

    ymax *= 1.1;

    if (ymax == 0)
        ymax = 1;

    Grafico g(
        "06_rule_based_regras.svg",
        "Operacoes do algoritmo rule-based",
        "Numero de celulas (N)",
        "Quantidade media"
    );

    g.grade(0, 650, 0, ymax);

    g.linha(
        s,
        0, 650, 0, ymax,
        "Singles",
        0
    );

    g.linha(
        t,
        0, 650, 0, ymax,
        "Tuples",
        1
    );
}

/*
============================================================
6. LOG-LOG / ANALISE ASSINTOTICA
============================================================
*/

void graficoLogLog(
    const vector<Resultado>& resultados
) {

    map<pair<string,int>, vector<double>> dados;

    for (const auto& r : resultados) {

        if (r.status != "solved")
            continue;

        if (r.tempo_ms <= 0)
            continue;

        dados[{r.algoritmo, r.celulas}]
            .push_back(r.tempo_ms);
    }

    map<string, vector<pair<double,double>>> series;

    double xmin = 100;
    double xmax = 1000;

    double ymin = 1e100;
    double ymax = -1e100;

    for (const auto& [chave, valores] : dados) {

        double tempo = media(valores);

        if (tempo <= 0)
            continue;

        double x = log10(
            static_cast<double>(chave.second)
        );

        double y = log10(tempo);

        series[chave.first].push_back({
            x,
            y
        });

        ymin = min(ymin, y);
        ymax = max(ymax, y);
    }

    ymin -= 0.2;
    ymax += 0.2;

    xmin = log10(10.0);
    xmax = log10(1000.0);

    Grafico g(
        "08_loglog_assintotico.svg",
        "Analise empirica do crescimento em escala log-log",
        "log10(N)",
        "log10(tempo em ms)"
    );

    g.grade(
        xmin,
        xmax,
        ymin,
        ymax
    );

    if (series.count("backtracking"))
        g.linha(
            series["backtracking"],
            xmin,
            xmax,
            ymin,
            ymax,
            "Backtracking",
            0
        );

    if (series.count("rule_based"))
        g.linha(
            series["rule_based"],
            xmin,
            xmax,
            ymin,
            ymax,
            "Rule-based",
            1
        );
}

/*
============================================================
7. RELATORIO DE COMPLEXIDADE EMPIRICA
============================================================
*/

void analisarComplexidade(
    const vector<Resultado>& resultados
) {

    map<string, map<int, vector<double>>> dados;

    for (const auto& r : resultados) {

        if (r.status != "solved")
            continue;

        if (r.tempo_ms <= 0)
            continue;

        dados[r.algoritmo][r.celulas]
            .push_back(r.tempo_ms);
    }

    cout << "\n";
    cout << "============================================\n";
    cout << " ANALISE EMPIRICA DE CRESCIMENTO\n";
    cout << "============================================\n";

    for (auto& [algoritmo, tamanhos] : dados) {

        vector<double> x;
        vector<double> y;

        for (auto& [n, valores] : tamanhos) {

            double tempo = media(valores);

            if (tempo <= 0)
                continue;

            x.push_back(log(
                static_cast<double>(n)
            ));

            y.push_back(log(tempo));
        }

        if (x.size() < 2)
            continue;

        double sx = 0;
        double sy = 0;
        double sxy = 0;
        double sx2 = 0;

        for (size_t i = 0; i < x.size(); i++) {

            sx += x[i];
            sy += y[i];
            sxy += x[i] * y[i];
            sx2 += x[i] * x[i];
        }

        double n = x.size();

        double denominador =
            n * sx2 - sx * sx;

        if (fabs(denominador) < 1e-12)
            continue;

        double alpha =
            (n * sxy - sx * sy) /
            denominador;

        cout << "\n";
        cout << algoritmo << ":\n";

        cout << "  Crescimento empirico aproximado:\n";

        cout << "  T(N) ~ N^"
             << fixed
             << setprecision(4)
             << alpha
             << "\n";
    }

    cout << "\n";
    cout << "IMPORTANTE:\n";
    cout << "O expoente acima e uma estimativa empirica.\n";
    cout << "Ele nao prova a complexidade teorica do algoritmo.\n";
    cout << "============================================\n\n";
}

/*
============================================================
MAIN
============================================================
*/

int main(int argc, char* argv[]) {

    string nomeArquivo = "resultados.csv";

    if (argc >= 2)
        nomeArquivo = argv[1];

    cout << "============================================\n";
    cout << " GERADOR DE GRAFICOS - SUDOKU\n";
    cout << "============================================\n\n";

    std::filesystem::create_directories("results/graficos");

    cout << "Lendo: "
         << nomeArquivo
         << "\n";

    vector<Resultado> resultados =
        lerCSV(nomeArquivo);

    cout << "Resultados encontrados: "
         << resultados.size()
         << "\n\n";

    if (resultados.empty()) {

        cerr << "Nenhum resultado encontrado.\n";

        return 1;
    }

    /*
     * Graficos principais
     */

    cout << "Gerando 01_tempo_medio.svg...\n";
    graficoTempo(resultados);

    cout << "Gerando 02_tempo_log.svg...\n";
    graficoTempoLog(resultados);

    cout << "Gerando 03_speedup.svg...\n";
    graficoSpeedup(resultados);

    cout << "Gerando 04_chamadas_recursivas.svg...\n";

    graficoMetricas(
        resultados,
        "recursive",
        "04_chamadas_recursivas.svg",
        "Chamadas recursivas do backtracking",
        "Chamadas recursivas"
    );

    cout << "Gerando 05_backtracks.svg...\n";

    graficoMetricas(
        resultados,
        "backtracks",
        "05_backtracks.svg",
        "Numero de backtracks",
        "Backtracks"
    );

    cout << "Gerando 06_rule_based_regras.svg...\n";

    graficoRuleBased(resultados);

    cout << "Gerando 08_loglog_assintotico.svg...\n";

    graficoLogLog(resultados);

    cout << "Gerando 10_guesses.svg...\n";

    graficoMetricas(
        resultados,
        "guesses",
        "10_guesses.svg",
        "Numero de guesses do backtracking",
        "Guesses"
    );

    /*
     * Analise numerica
     */

    analisarComplexidade(resultados);

    cout << "\n============================================\n";
    cout << " GRAFICOS GERADOS COM SUCESSO\n";
    cout << "============================================\n\n";

    cout << "Arquivos:\n";

    cout << "  01_tempo_medio.svg\n";
    cout << "  02_tempo_log.svg\n";
    cout << "  03_speedup.svg\n";
    cout << "  04_chamadas_recursivas.svg\n";
    cout << "  05_backtracks.svg\n";
    cout << "  06_rule_based_regras.svg\n";
    cout << "  08_loglog_assintotico.svg\n";
    cout << "  10_guesses.svg\n";

    cout << "\nAbra os arquivos .svg no navegador.\n";

    return 0;
}
