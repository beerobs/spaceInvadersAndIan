#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

/* ---------------------------- */
/*           DEFINES            */
/* ---------------------------- */
#define EPSILON 1e-9

#define LARGURA_JANELA 800
#define ALTURA_JANELA 600
#define LARGURA_UTIL (LARGURA_JANELA-20)
#define ALTURA_UTIL (ALTURA_JANELA-20)

#define STD_SIZE_X 32
#define STD_SIZE_Y 32
#define ESPACAMENTO_NAVE_X 16
#define ESPACAMENTO_NAVE_Y 10
#define SPACE_X (STD_SIZE_X + ESPACAMENTO_NAVE_X)
#define SPACE_Y (STD_SIZE_Y + ESPACAMENTO_NAVE_Y)

#define MAX_LINHAS 6
#define MIN_LINHAS 4
#define MAX_NAVE_LINHA 11
#define MIN_NAVE_LINHA 7
#define MAX_NAVE MAX_LINHAS * MAX_NAVE_LINHA

#define CHANCE_DE_TIRO 20
#define MAX_DIFICULDADE 9
#define LARGURA_BALA 10
#define ALTURA_BALA 15

#define QTD_BARREIRAS 3

#define IMUNE (j->tempoAnimacao - j->heroi.tempoImunidade <= 2)

/* ---------------------------- */
/*         ESTRUTURAS           */
/* ---------------------------- */
typedef struct Bala {
    Rectangle pos;
    Color color;
    int ativa;
    int tempo;
    int intervalo_de_tiro;
    int velocidade;
} Bala;

typedef struct Barreira {
    Rectangle pos;
    Color cor;
    int ativa;
    int vida;
    int direcao;
} Barreira;

typedef struct BalaHeroi {
    Rectangle pos;
    Color color;
    int ativa;
    int tempo;
    int velocidade;
    Sound tiro;
} BalaHeroi;

typedef struct Naves {
    Color color;
    Bala bala;
    double velocidade;
    int direcao;
    int chanceDeTiroBase;
    Sound tiro;
    Sound explosaoNaves;
} Naves;

typedef struct Nave {
    Rectangle pos;
    Color color;
    Bala bala;
    int ativa;
    int chanceDeTiro;
} Nave;

typedef struct Heroi {
    Rectangle pos;
    Color color;
    BalaHeroi bala;
    Sound colisaoHeroi;
    int velocidade;
    int direcao;
    int vidas;
    int tempoImunidade;

    int desaparece;
    double tempoDesaparece;
} Heroi;

typedef struct Bordas {
    Rectangle pos;
} Bordas;

typedef struct Assets {
    Texture2D naveVerde;
    Texture2D heroiPrata;
    Texture2D telaFinal;
    Sound tiro;
    Sound transicao;
} Assets;

typedef struct Rodada {
    int numeroRodadas;
    double dificuldade;
    int navesPorLinha;
    int linhasDeNaves;
    int qtdNave;

    int acrescimoVel[4];
    int acrescimoTiro[4];

    double maxTempoRodada;
    double inicioRodada;

} Rodada;

typedef struct Interface {
    char textoVidas[20];
    char textoTempo[20];
    char textoPontuacao[30];
    char textoRodada[30];
} Interface;

typedef struct Jogo {
    Naves naves;
    Nave nave[MAX_LINHAS][MAX_NAVE_LINHA];
    Heroi heroi;
    Bordas bordas[4];
    Rodada rodada;
    Barreira barreiras[QTD_BARREIRAS];
        int decrementoCorBarreira;
        int barreiras_dinamicas;
    Assets assets;
    Interface interface;
    Music musica;

    int alturaJanela;
    int larguraJanela;

    int tempoAnimacao;
    int pontuacao;
    int congela;
    int tempoCongela;

    char** ranking;
    int* pontuacoes;
    char nomeJogador[10];
    FILE* arquivo;

} Jogo;

/* ---------------------------- */
/*      PROTÓTIPOS DE FUNÇÕES   */
/* ---------------------------- */

/* Funções de Inicialização */
void IniciaJogo(Jogo *j);
void IniciaNaves(Jogo *j);
void IniciaBarreiras(Jogo *j);
void RandomizaPosicaoNave(Jogo *j);
void CarregaImagens(Jogo *j);
void DescarregaImagens(Jogo *j);

/* Funções de Atualização */
void AtualizaJogo(Jogo *j);
void AtualizaFrameDesenho(Jogo *j);
void AtualizaRodada(Jogo* j);
void AtualizaNavePos(Jogo *j);
void AtualizaHeroiPos(Jogo *j);
void AtualizaVelocidadeNaves(Jogo* j);
void AtualizaChanceDeTiroNaves(Jogo* j);
void AtualizaBarreiras(Jogo *j);

/* Funções de Desenho */
void DesenhaJogo(Jogo *j);
void DesenhaInterface(Jogo *j);
void DesenhaNave(Jogo *j);
void DesenhaHeroi(Jogo *j);
void DesenhaBordas(Jogo *j);
void DesenhaBalas(Jogo *j);
void DesenhaBalasHeroi(Jogo *j);
void DesenhaRanking(Jogo* j);
void DesenhaVidas(Jogo *j);
void DesenhaTempo(Jogo *j);
void DesenhaPontuacao(Jogo* j);
void DesenhaRodada(Jogo* j);
void DesenhaBarreiras(Jogo *j);

/* Funções de Tiro e Colisão */
void AtiraBalas(Jogo *j);
void AtiraBalasHeroi(Jogo *j);
void ColisaoBordas(Jogo *j);
int ColisaoBalas(Jogo *j, int i, int k);
int ColisaoBalasHeroi(Jogo *j);

/* Funções de Ranking e Tela Inicial */
void TelaInicial(Jogo* j);
void DescarregaRanking(Jogo* j);
void SalvarRanking(Jogo* j);
int VerificaTop(Jogo* j);
void solicitarNome(char *nomeJogador);

/* Funções Auxiliares */
int FinalDeJogo(Jogo* j);
int TelaDerrota(Jogo* j);
int DoubleEquals(double a, double b);

/* ---------------------------- */
/*            MAIN              */
/* ---------------------------- */
int main(){
    InitAudioDevice();
    
    srand(time(NULL));
    
    Jogo jogo = {};

    jogo.alturaJanela = ALTURA_JANELA;
    jogo.larguraJanela = LARGURA_JANELA;

    InitWindow(jogo.larguraJanela, jogo.alturaJanela, "Space Invaders");
    SetTargetFPS(60);
    CarregaImagens(&jogo);
    jogo.musica = LoadMusicStream("../assets/musica.mp3");
    SetMusicVolume(jogo.musica, 0.5f);
    PlayMusicStream(jogo.musica);
    jogo.assets.transicao = LoadSound("../assets/transicao.wav");
    SetSoundVolume(jogo.assets.transicao, 1.0f);
    
    TelaInicial(&jogo);

    while(!WindowShouldClose() && !FinalDeJogo(&jogo)){
        UpdateMusicStream(jogo.musica);
        AtualizaFrameDesenho(&jogo);
    }

    UnloadMusicStream(jogo.musica);
    DescarregaImagens(&jogo);
    CloseWindow();

    for(int i=0; i<10; i++) free(jogo.ranking[i]);
    free(jogo.ranking);
    free(jogo.pontuacoes);
    if(jogo.arquivo != NULL) fclose(jogo.arquivo);
    return 0;
}

/* ---------------------------- */
/*  FUNÇÕES DE INICIALIZAÇÃO    */
/* ---------------------------- */
void IniciaJogo(Jogo *j){
    j->tempoAnimacao = GetTime();
    j->rodada.numeroRodadas = 1;
    j->rodada.dificuldade = 1;
    #define TEMPO_RODADA 60
    j->rodada.maxTempoRodada = TEMPO_RODADA;
    j->pontuacao = 0;
    j->congela = 1;
    j->tempoCongela = j->tempoAnimacao;

    j->rodada.acrescimoVel[0] = 0;
    j->rodada.acrescimoVel[1] = 0;
    j->rodada.acrescimoVel[2] = 0;
    j->rodada.acrescimoVel[3] = 0;

    j->rodada.acrescimoTiro[0] = 0;
    j->rodada.acrescimoTiro[1] = 0;
    j->rodada.acrescimoTiro[2] = 0;
    j->rodada.acrescimoTiro[3] = 0;

    j->heroi.pos = (Rectangle) {LARGURA_JANELA/2 - STD_SIZE_X/2, ALTURA_JANELA - STD_SIZE_Y -13, STD_SIZE_X, STD_SIZE_Y};
    j->heroi.color = PURPLE;
    j->heroi.velocidade = 3;
    j->heroi.bala.ativa = 0;
    j->heroi.bala.tempo = GetTime();
    j->heroi.bala.velocidade = 10;
    j->heroi.bala.tiro = LoadSound("../assets/shoot.wav");
    SetSoundVolume(j->heroi.bala.tiro, 0.3f);
    j->heroi.colisaoHeroi = LoadSound("../assets/colisao_heroi.wav");
    j->heroi.vidas = 3;
    j->heroi.tempoImunidade = j->tempoAnimacao+1;
    j->heroi.desaparece = 0;
    
    IniciaNaves(j);
    IniciaBarreiras(j);

    // Borda superior
    j->bordas[0].pos = (Rectangle){0, 0, LARGURA_JANELA, 10};
    // Borda inferior
    j->bordas[1].pos = (Rectangle){0, ALTURA_JANELA-10, LARGURA_JANELA, 10};
    // Borda esquerda
    j->bordas[2].pos = (Rectangle){0, 0, 10, ALTURA_JANELA};
    // Borda direita
    j->bordas[3].pos = (Rectangle){LARGURA_JANELA-10, 0, 10, ALTURA_JANELA};

}

void IniciaNaves(Jogo *j){
    
    j->rodada.navesPorLinha = GetRandomValue(MIN_NAVE_LINHA, MAX_NAVE_LINHA);
    j->rodada.linhasDeNaves = GetRandomValue(MIN_LINHAS, MAX_LINHAS);
    j->rodada.qtdNave = j->rodada.navesPorLinha * j->rodada.linhasDeNaves;
    j->naves.chanceDeTiroBase = CHANCE_DE_TIRO + (j->rodada.numeroRodadas-1);
    
    // Inicializa a matriz de naves
    int numAleatorio = 0;
    int numAnterior = -1;
    Color corAleatoria = {};
    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        // Estabelece a cor aleatória da linha de naves, não repetindo a cor da linha anterior.
        while(numAleatorio == numAnterior) 
            numAleatorio = rand() % 4;
        if(numAleatorio == 3)
            corAleatoria = GRAY;
        else if(numAleatorio == 2) 
            corAleatoria = BLUE;
        else if(numAleatorio == 1) 
            corAleatoria = PINK;
        else if(numAleatorio == 0) 
            corAleatoria = PURPLE;
        numAnterior = numAleatorio;
        
        for(int k = 0; k < j->rodada.navesPorLinha; k++){
            j->nave[i][k].color = corAleatoria;
            j->nave[i][k].ativa = 1;
            j->nave[i][k].bala.ativa = 0;
            j->nave[i][k].chanceDeTiro = j->naves.chanceDeTiroBase + (j->rodada.dificuldade) * (1.2 + j->rodada.linhasDeNaves - i);
            j->nave[i][k].chanceDeTiro = j->nave[i][k].chanceDeTiro > 90? 85: j->nave[i][k].chanceDeTiro; 
            j->nave[i][k].bala.tempo = GetTime();
        }
    }

    RandomizaPosicaoNave(j);

    j->naves.velocidade = j->rodada.dificuldade <9? 1 + (double)j->rodada.dificuldade/4 : j->naves.velocidade;
    /* direcao = 1 -> mover para direita, direcao = 0 -> mover para esquerda */
    j->naves.direcao = 1;
    j->naves.bala.ativa = 0;
    j->naves.bala.velocidade = 5;
    if(j->rodada.numeroRodadas == 1){
        j->naves.tiro = LoadSound("../assets/shoot.wav");
        SetSoundVolume(j->naves.tiro, 0.3f);
        j->naves.explosaoNaves = LoadSound("../assets/explosao_naves.wav");
        SetSoundVolume(j->naves.explosaoNaves, 0.6f);
    }
}

void IniciaBarreiras(Jogo *j) {
    int larguraBarreira = 80 - GetRandomValue(0, (j->rodada.numeroRodadas-1)*9);
    if(larguraBarreira < 20) {
        for(int i = 0; i < 3; i++) j->barreiras[i].ativa = 0;
        return;
    }
    int vidaBase = 2;
    int incrementoVida = j->rodada.navesPorLinha;
    int vidaFinal = vidaBase + incrementoVida;
    j->decrementoCorBarreira = 255/(vidaFinal+5);
    float posY[] = { ALTURA_JANELA / 2 + 225, ALTURA_JANELA / 2 + 200, ALTURA_JANELA / 2 + 175 };

    int chave = GetRandomValue(1, 100);
    // Ativa Barreiras dinâmicas se a chave for menor que dificuldade*10
    if(chave <= (j->rodada.dificuldade-1)*10){
        j->barreiras_dinamicas = 1;
        for (int i = 0; i < 3; i++) {
            float posX = GetRandomValue(10 + larguraBarreira / 2, LARGURA_JANELA - larguraBarreira / 2);
            j->barreiras[i].cor = BROWN;
    
            for (int k = 0; k < i; k++) {
                while (abs(j->barreiras[k].pos.x - posX) < 100) {
                    posX = GetRandomValue(10 + larguraBarreira / 2, LARGURA_JANELA - larguraBarreira / 2);
                }
            }
    
            j->barreiras[i].pos = (Rectangle) {posX - larguraBarreira / 2, posY[i], larguraBarreira, 15};
            j->barreiras[i].ativa = 1;
            j->barreiras[i].vida = vidaFinal;
            j->barreiras[i].direcao = GetRandomValue(0, 1);
            
            if((CheckCollisionRecs(j->barreiras[i].pos, j->bordas[2].pos) || CheckCollisionRecs(j->barreiras[i].pos, j->bordas[3].pos))){
                i--;
            }
        }
    } else {
        #define ESPACAMENTO_DA_BORDA 170
        j->barreiras_dinamicas = 0;
        j->barreiras[0].pos = (Rectangle) {ESPACAMENTO_DA_BORDA - larguraBarreira / 2, ALTURA_JANELA / 2 + 225, larguraBarreira, 15};
        j->barreiras[1].pos = (Rectangle) {LARGURA_JANELA/2 - larguraBarreira/2, ALTURA_JANELA / 2 + 225, larguraBarreira, 15};
        j->barreiras[2].pos = (Rectangle) {(LARGURA_JANELA-ESPACAMENTO_DA_BORDA) - larguraBarreira / 2, ALTURA_JANELA / 2 + 225, larguraBarreira, 15};
        for(int i = 0; i < 3; i++){
            j->barreiras[i].ativa = 1;
            j->barreiras[i].vida = vidaFinal;
            j->barreiras[i].cor = BROWN;
        }
    }
}

void RandomizaPosicaoNave(Jogo *j){
    int posPrimeiraColuna = 16;
    int posPrimeiraLinha  = (ALTURA_JANELA-10) - ((SPACE_Y*(4.5 - j->rodada.dificuldade/6)) + SPACE_Y * (j->rodada.linhasDeNaves + GetRandomValue(0, 7 - j->rodada.linhasDeNaves)));    

    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        for(int k = 0; k < j->rodada.navesPorLinha; k++){
            j->nave[i][k].pos = (Rectangle){(posPrimeiraColuna + ESPACAMENTO_NAVE_X*k + STD_SIZE_X * k), (posPrimeiraLinha + ESPACAMENTO_NAVE_Y*i + STD_SIZE_Y * i), STD_SIZE_X, STD_SIZE_Y};
        } 
    }
}

void CarregaImagens(Jogo *j){
    j->assets.naveVerde = LoadTexture("../assets/GreenAnimation.png");
    j->assets.heroiPrata = LoadTexture("../assets/SilverStatic.png");
}

void DescarregaImagens(Jogo *j){
    UnloadTexture(j->assets.naveVerde);
    UnloadTexture(j->assets.heroiPrata);
}

/* ---------------------------- */
/*   FUNÇÕES DE ATUALIZAÇÃO     */
/* ---------------------------- */
void AtualizaJogo(Jogo *j){
    j->tempoAnimacao = GetTime();
    
    AtualizaRodada(j);

    #define TEMPO_CONGELAMENTO 1
    if(j->congela && j->tempoAnimacao - j->tempoCongela > TEMPO_CONGELAMENTO){ 
        j->congela = 0;
        j->rodada.inicioRodada = j->tempoAnimacao;
    }
    
    if(j->congela) return;
    AtualizaNavePos(j);
    AtualizaHeroiPos(j);
    AtualizaBarreiras(j);
    AtiraBalas(j);
    AtiraBalasHeroi(j);
}

void AtualizaFrameDesenho(Jogo *j){
    AtualizaJogo(j);
    DesenhaJogo(j);
}

void AtualizaRodada(Jogo *j){
    
    AtualizaVelocidadeNaves(j);
    AtualizaChanceDeTiroNaves(j);

    if(j->rodada.qtdNave<=0 || (j->tempoAnimacao - j->rodada.inicioRodada) > j->rodada.maxTempoRodada || IsKeyPressed(KEY_H)){
        
        j->heroi.bala.ativa = 0;
        for(int i = 0; i < 4; i++) j->rodada.acrescimoVel[i] = 0;
        for(int i = 0; i < 4; i++) j->rodada.acrescimoTiro[i] = 0;
        j->rodada.numeroRodadas++;
        j->rodada.maxTempoRodada = TEMPO_RODADA;
        j->rodada.inicioRodada = j->tempoAnimacao;
        j->rodada.dificuldade += j->rodada.dificuldade < 9? 1 : 0;

        
        IniciaNaves(j);
        IniciaBarreiras(j);

        j->heroi.pos.x = LARGURA_JANELA/2;
        j->heroi.tempoImunidade = j->tempoAnimacao;

        j->congela = 1;
        j->tempoCongela = j->tempoAnimacao;

        PlaySound(j->assets.transicao);
    }
    return;
}

void AtualizaNavePos(Jogo *j){
    ColisaoBordas(j);

    if(j->naves.direcao == 1){
        for(int i = 0; i < j->rodada.linhasDeNaves; i++){
            for(int k = 0; k < j->rodada.navesPorLinha; k++){
                j->nave[i][k].pos.x += j->naves.velocidade;
            }
        }
    } else {
        for(int i = 0; i < j->rodada.linhasDeNaves; i++){
            for(int k = 0; k < j->rodada.navesPorLinha; k++){
                j->nave[i][k].pos.x -= j->naves.velocidade;
            }
        }
    }
}

void AtualizaHeroiPos(Jogo *j){
    if(IsKeyDown(KEY_RIGHT) && j->heroi.pos.x < LARGURA_JANELA-STD_SIZE_X - 10){
        j->heroi.pos.x += j->heroi.velocidade;
    } else if(IsKeyDown(KEY_LEFT) && j->heroi.pos.x > 10){
        j->heroi.pos.x -= j->heroi.velocidade;
    }
}

void AtualizaBarreiras(Jogo *j) {
    if(!j->barreiras_dinamicas){
        return;
    }

    for (int b = 0; b < 3; b++) {
        if(j->barreiras[b].ativa && CheckCollisionRecs(j->barreiras[b].pos, j->bordas[2].pos)){
            j->barreiras[b].direcao = 1;
        } else if(j->barreiras[b].ativa && CheckCollisionRecs(j->barreiras[b].pos, j->bordas[3].pos)){
            j->barreiras[b].direcao = 0;
        }
        j->barreiras[b].pos.x += j->barreiras[b].ativa && j->barreiras[b].direcao==1 ? 1 : j->barreiras[b].ativa? -1 : 0;
    }
    
}

void AtualizaVelocidadeNaves(Jogo* j){
    if(j->rodada.maxTempoRodada - (j->tempoAnimacao - j->rodada.inicioRodada) <= j->rodada.maxTempoRodada*0.50 && !j->rodada.acrescimoVel[1]){
        j->naves.velocidade++;
        j->rodada.acrescimoVel[1] = 1;
    } else if(j->rodada.maxTempoRodada - (j->tempoAnimacao - j->rodada.inicioRodada) <= j->rodada.maxTempoRodada*0.25 && !j->rodada.acrescimoVel[2]){
        j->naves.velocidade++;
        j->rodada.acrescimoVel[2] = 1;
    } else if(j->rodada.maxTempoRodada - (j->tempoAnimacao - j->rodada.inicioRodada) <= j->rodada.maxTempoRodada*0.12 && !j->rodada.acrescimoVel[3]){
        j->naves.velocidade++;
        j->rodada.acrescimoVel[3] = 1;
    }
}

void AtualizaChanceDeTiroNaves(Jogo* j){
    if(j->rodada.maxTempoRodada - (j->tempoAnimacao - j->rodada.inicioRodada) <= j->rodada.maxTempoRodada*0.75 && !j->rodada.acrescimoTiro[0]){
        for(int i = 0;i < j->rodada.linhasDeNaves; i++){
            for(int k = 0; k < j->rodada.navesPorLinha; k++){
                j->nave[i][k].chanceDeTiro += 5;
                j->rodada.acrescimoTiro[0] = 1;
            }
        }
    } else if(j->rodada.maxTempoRodada - (j->tempoAnimacao - j->rodada.inicioRodada) <= j->rodada.maxTempoRodada*0.50 && !j->rodada.acrescimoTiro[1]){
        for(int i = 0;i < j->rodada.linhasDeNaves; i++){
            for(int k = 0; k < j->rodada.navesPorLinha; k++){
                j->nave[i][k].chanceDeTiro += 15;
                j->rodada.acrescimoTiro[1] = 1;
            }
        }
    } else if(j->rodada.maxTempoRodada - (j->tempoAnimacao - j->rodada.inicioRodada) <= j->rodada.maxTempoRodada*0.25 && !j->rodada.acrescimoTiro[2]){
        for(int i = 0;i < j->rodada.linhasDeNaves; i++){
            for(int k = 0; k < j->rodada.navesPorLinha; k++){
                j->nave[i][k].chanceDeTiro += 20;
                j->rodada.acrescimoTiro[2] = 1;
            }
        }
    } else if(j->rodada.maxTempoRodada - (j->tempoAnimacao - j->rodada.inicioRodada) <= j->rodada.maxTempoRodada*0.12 && !j->rodada.acrescimoTiro[3]){
        for(int i = 0;i < j->rodada.linhasDeNaves; i++){
            for(int k = 0; k < j->rodada.navesPorLinha; k++){
                j->nave[i][k].chanceDeTiro += 20;
                j->rodada.acrescimoTiro[3] = 1;
            }
        }
    }
}

/* ---------------------------- */
/*    FUNÇÕES DE DESENHO        */
/* ---------------------------- */
void DesenhaJogo(Jogo *j){
    BeginDrawing();
    ClearBackground(BLACK);
    DesenhaNave(j);
    DesenhaHeroi(j);
    DesenhaBordas(j);
    DesenhaBarreiras(j);
    DesenhaInterface(j);
    EndDrawing();
}

void DesenhaInterface(Jogo* j){
    DesenhaTempo(j);
    DesenhaPontuacao(j);
    DesenhaRodada(j);
    DesenhaVidas(j);
}

void DesenhaNave(Jogo *j){
    Vector2 tamanhoFrame = {32, 32};
    
    static Vector2 frame = {0, 0};
    static float tempoUltimaTroca = 0;
    
    if(GetTime() - tempoUltimaTroca >= 1){
        if(frame.x == 0){
            frame.x = 1;
        } else {
            frame.x = 0;
        }
        tempoUltimaTroca = GetTime();
    }
    Rectangle frameRecNave = {frame.x * tamanhoFrame.x, frame.y * tamanhoFrame.y,
                              tamanhoFrame.x, tamanhoFrame.y};

    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        for(int k = 0; k < j->rodada.navesPorLinha; k++)
            if(j->nave[i][k].ativa)
                DrawTexturePro(
                    j->assets.naveVerde, 
                    frameRecNave, 
                    (Rectangle){j->nave[i][k].pos.x, j->nave[i][k].pos.y, 32, 32}, 
                    (Vector2){0, 0}, 
                    0.0f, 
                    j->nave[i][k].color
                );
    }
}

void DesenhaHeroi(Jogo *j){
    if(IMUNE){
        #define INTERVALO_FADE 0.15

        if(!j->heroi.desaparece) 
            DrawTexture(j->assets.heroiPrata, j->heroi.pos.x, j->heroi.pos.y, j->heroi.color);

        if(j->heroi.desaparece && GetTime() - j->heroi.tempoDesaparece > INTERVALO_FADE - EPSILON){
            j->heroi.desaparece = 0;
            j->heroi.tempoDesaparece = GetTime();
        }
        else if(!j->heroi.desaparece && GetTime() - j->heroi.tempoDesaparece > INTERVALO_FADE - EPSILON){
            j->heroi.desaparece = 1;
            j->heroi.tempoDesaparece = GetTime();
        }
    } else {
        DrawTexture(j->assets.heroiPrata, j->heroi.pos.x, j->heroi.pos.y, j->heroi.color);
    }
}

void DesenhaBordas(Jogo *j){
    for(int i = 0; i < 4; i++){
        DrawRectangleRec(j->bordas[i].pos, LIGHTGRAY);
    }
}

void DesenhaBalas(Jogo *j){
    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        for(int k = 0; k < j->rodada.navesPorLinha; k++){
            if(j->nave[i][k].bala.ativa)
                DrawRectangleRec(j->nave[i][k].bala.pos, YELLOW);            
        }
    }
}

void DesenhaBalasHeroi(Jogo *j){
    DrawRectangleRec(j->heroi.bala.pos, RED);
}

void DesenhaRanking(Jogo* j){
    static char buf[100] = {};
    #define TAM_FONTE 18
    #define ESPACAMENTO_NOME_PONTUACAO 250
    #define POS_Y 320
    #define ESPACAMENTO_ENTRE_NOMES 6
    int espacamentoLugar = MeasureText("02.  ", TAM_FONTE);
    int comprimento = ESPACAMENTO_NOME_PONTUACAO + MeasureText("000000", TAM_FONTE) + espacamentoLugar;
    for(int i = 0; i < 10; i++){
        sprintf(buf, "%02d.", i+1);
        DrawText(buf, LARGURA_JANELA/2 - comprimento/2, POS_Y + i*(TAM_FONTE+ESPACAMENTO_ENTRE_NOMES), TAM_FONTE, LIGHTGRAY);
        DrawText(j->ranking[i], espacamentoLugar + LARGURA_JANELA/2 - comprimento/2, POS_Y + i*(TAM_FONTE+ESPACAMENTO_ENTRE_NOMES), TAM_FONTE, LIGHTGRAY);
        sprintf(buf, "%6d", j->pontuacoes[i]);
        DrawText(buf, (LARGURA_JANELA/2 - comprimento/2) + ESPACAMENTO_NOME_PONTUACAO, POS_Y + i*(TAM_FONTE+ESPACAMENTO_ENTRE_NOMES), TAM_FONTE, LIGHTGRAY);
    }
}

void DesenhaVidas(Jogo *j) {
    sprintf(j->interface.textoVidas, "V: %d", j->heroi.vidas);
    DrawText(j->interface.textoVidas, 20, 20, 20, LIGHTGRAY);
}

void DesenhaTempo(Jogo *j){
    int tempo = !j->congela ? (int)j->rodada.maxTempoRodada - (int)(GetTime() - j->rodada.inicioRodada) : j->rodada.maxTempoRodada;
    sprintf(j->interface.textoTempo, "%d", tempo);
    DrawText(j->interface.textoTempo, LARGURA_JANELA-40, 20, 20, LIGHTGRAY);
}

void DesenhaPontuacao(Jogo* j){
    sprintf(j->interface.textoPontuacao, "%010d", j->pontuacao);
    DrawText(j->interface.textoPontuacao, LARGURA_JANELA/2.5 - 50, 20, 20, LIGHTGRAY);
}

void DesenhaRodada(Jogo* j){
    sprintf(j->interface.textoRodada, "RODADA: %d", j->rodada.numeroRodadas);
    DrawText(j->interface.textoRodada, LARGURA_JANELA/1.8, 20, 20, LIGHTGRAY);
}

void DesenhaBarreiras(Jogo *j) {
    for(int i = 0; i < 3; i++) {
        if(j->barreiras[i].ativa)
            DrawRectangleRec(j->barreiras[i].pos, j->barreiras[i].cor);
    }
}

int TelaDerrota(Jogo* j){
    j->assets.telaFinal = LoadTexture("../assets/YouLose.png");
    while(IsKeyUp(KEY_ESCAPE) && !WindowShouldClose()){
        BeginDrawing();
        DrawTexture(j->assets.telaFinal, 0, 0, WHITE);
        if(GetKeyPressed() == KEY_ENTER){
            SeekMusicStream(j->musica, 0.0f);
            TelaInicial(j);
            UnloadTexture(j->assets.telaFinal);
            return 0;
        } 

        EndDrawing();
    }
    UnloadTexture(j->assets.telaFinal);
    return 1;
}

/* ---------------------------- */
/*   FUNÇÕES DE TIRO E COLISÃO  */
/* ---------------------------- */
void AtiraBalas(Jogo *j){
    for(int k = 0; k < j->rodada.navesPorLinha; k++){
        for(int i = j->rodada.linhasDeNaves-1; i >= 0; i--){
            if(i != j->rodada.linhasDeNaves-1 && j->nave[i+1][k].ativa)
                break;
            
            // Atira a bala se pronta e passar na probabilidade
            if(j->nave[i][k].ativa && !j->nave[i][k].bala.ativa && GetTime()-j->nave[i][k].bala.tempo > 1){
                if(GetRandomValue(0,100) > j->nave[i][k].chanceDeTiro || j->congela){
                    j->nave[i][k].bala.tempo = GetTime();
                    break;
                }
                
                j->nave[i][k].bala.pos = (Rectangle){j->nave[i][k].pos.x+j->nave[i][k].pos.width/2, j->nave[i][k].pos.y+j->nave[i][k].pos.height/2, 
                                                      LARGURA_BALA, ALTURA_BALA};
                j->nave[i][k].bala.ativa = 1;
                PlaySound(j->naves.tiro);
            }
            else if(ColisaoBalas(j, i, k) || j->congela){
                j->nave[i][k].bala.ativa = 0;
                j->nave[i][k].bala.pos = (Rectangle){-33, -976};
            }
            if(j->nave[i][k].bala.ativa == 1){
                j->nave[i][k].bala.pos.y += j->naves.bala.velocidade;
            }
        }
    }
    DesenhaBalas(j);
}

void AtiraBalasHeroi(Jogo *j){
    if(j->heroi.bala.ativa == 0 && IsKeyDown(KEY_SPACE) && !j->congela && !IMUNE){
        j->heroi.bala.pos = (Rectangle){j->heroi.pos.x+j->heroi.pos.width/2, j->heroi.pos.y+j->heroi.pos.height/2, 
                                        LARGURA_BALA, ALTURA_BALA};
        j->heroi.bala.ativa = 1;
        j->heroi.bala.tempo = GetTime();
        PlaySound(j->heroi.bala.tiro);
    }
    else if(ColisaoBalasHeroi(j) || j->congela){
        j->heroi.bala.ativa = 0;
        j->heroi.bala.pos = (Rectangle){999, 999};
    }
    if(j->heroi.bala.ativa == 1){
        j->heroi.bala.pos.y -= j->heroi.bala.velocidade;
        DesenhaBalasHeroi(j);
    }
}

void ColisaoBordas(Jogo *j){
    if(CheckCollisionRecs(j->nave[0][0].pos, j->bordas[2].pos)){
        j->naves.direcao = 1;
    } else if(CheckCollisionRecs(j->nave[0][j->rodada.navesPorLinha-1].pos, j->bordas[3].pos)){
        j->naves.direcao = 0;
    }
}

int ColisaoBalas(Jogo *j, int i, int k){
    if(CheckCollisionRecs(j->heroi.pos, j->nave[i][k].bala.pos)){
        if(!IMUNE){
            j->heroi.vidas--;
            PlaySound(j->heroi.colisaoHeroi);
            j->heroi.tempoImunidade = j->tempoAnimacao;
            j->heroi.desaparece = 0;
            j->heroi.tempoDesaparece = GetTime();
        }
        return 1;
    }
    if(CheckCollisionRecs(j->nave[i][k].bala.pos, j->bordas[1].pos)){
        return 1;
    }
    for (int b = 0; b < QTD_BARREIRAS; b++) {
        if (j->barreiras[b].ativa && CheckCollisionRecs(j->nave[i][k].bala.pos, j->barreiras[b].pos) && j->nave[i][k].bala.ativa) {
            j->nave[i][k].bala.ativa = 0;
            j->nave[i][k].bala.pos.x = 999;
            j->nave[i][k].bala.pos.y = 999;
            j->barreiras[b].vida--;
            j->barreiras[b].cor.a -= j->decrementoCorBarreira;

            if (j->barreiras[b].vida == 0) {
                j->barreiras[b].ativa = 0;
            }
        }
    }
    return 0;
}

int ColisaoBalasHeroi(Jogo *j){
    for(int i = 0; i < j->rodada.linhasDeNaves; i++){
        for(int k = 0; k < j->rodada.navesPorLinha; k++){
            if(CheckCollisionRecs(j->nave[i][k].pos, j->heroi.bala.pos) && j->nave[i][k].ativa){
                PlaySound(j->naves.explosaoNaves);
                j->nave[i][k].ativa = 0;
                j->rodada.qtdNave--;
                j->pontuacao += 10;
                return 1;
            }
        }
    }
    if(CheckCollisionRecs(j->heroi.bala.pos, j->bordas[0].pos)){
        return 1;
    }
    for (int i = 0; i < 3; i++) {
        if (CheckCollisionRecs(j->heroi.bala.pos, j->barreiras[i].pos) && j->barreiras[i].ativa) {
            j->heroi.bala.ativa = 0;
            j->heroi.bala.pos = (Rectangle){986, 1063};
        }
    }
    return 0;
}

/* ---------------------------- */
/*   FUNÇÕES DE RANKING/TELA    */
/* ---------------------------- */
void TelaInicial(Jogo *j){
    DescarregaRanking(j);
    int fade = 0;
    double fadeTime = GetTime();
    strcpy(j->nomeJogador, "___");
    char displayNome[20] = "___";
    int tamNome = 0;
    int key = 0;
    
    while(!WindowShouldClose()){
        UpdateMusicStream(j->musica);
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("INVASORES ESPACIAIS", (LARGURA_JANELA - MeasureText("INVASORES ESPACIAIS", 50))/2, 50, 50, LIGHTGRAY);
        if(!fade){
            DrawText("PRESSIONE SEU NOME E ENTER PARA JOGAR", (LARGURA_JANELA - MeasureText("PRESSIONE SEU NOME E ENTER PARA JOGAR", 30))/2, 160, 30, WHITE);
            if(GetTime() - fadeTime > 2.5){
                fade = 1;
                fadeTime = GetTime();
            }
        }else  if(GetTime() - fadeTime > 0.4){
            fade = 0;
            fadeTime = GetTime();
        }

        sprintf(displayNome, "%c%c%c", j->nomeJogador[0], j->nomeJogador[1], j->nomeJogador[2]);
        DrawText(displayNome, (LARGURA_JANELA - MeasureText(displayNome, 40))/2, 210, 40, LIGHTGRAY);

        DrawText("RANKING", (LARGURA_JANELA - MeasureText("RANKING", 20))/2, 280, 20, LIGHTGRAY);
        DesenhaRanking(j);

        key = GetKeyPressed();
        if(key == KEY_ENTER && tamNome >= 3){
            EndDrawing();
            IniciaJogo(j);            
            return;
        } else if(key == KEY_BACKSPACE && tamNome > 0){
            j->nomeJogador[--tamNome] = '_';
        }
        key = toupper(key);
        if(key >= 'A' && key <= 'Z' && tamNome < 3){
            j->nomeJogador[tamNome++] = key;
        }
        

    EndDrawing();
    }
    return;
}

void DescarregaRanking(Jogo *j){
    char** ranking = (char**)malloc(sizeof(char*)*10);
    for(int i = 0; i < 10; i++){
        ranking[i] = (char*)malloc(sizeof(char)*100);
        for(int j = 0; j < 100; j++){
            ranking[i][j] = '\0';
        }
    }
    int* pontuacoes = (int*)malloc(sizeof(int)*10);
    FILE* fp = fopen("../doc/ranking.txt", "r");
    if(fp == NULL){
        printf("Moiou! O arquivo \"ranking.txt\" não foi encontrado.\n");
        exit(1);
    }
    char buf[100] = {};
    int i = 0;
    while(fgets(buf, 100, fp)!= NULL){
        pontuacoes[i] = atoi(strtok(buf, "|"));
        strcpy(ranking[i], strtok(NULL, "|"));
        ranking[i][strcspn(ranking[i], "\n")] = '\0';
        i++;
    }
    
    fclose(fp);

    j->pontuacoes = pontuacoes;
    j->ranking = ranking;
}

void solicitarNome(char *nomeJogador) {
    int tamanhoNome = 0;
    nomeJogador[0] = '\0';
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Parabéns! Você entrou no top 10! Digite seu nome:", 100, 100, 20, LIGHTGRAY);
        DrawText(nomeJogador, 100, 150, 20, LIGHTGRAY);
        int key = GetKeyPressed();
        while (key > 0) {
            if (key == KEY_BACKSPACE && tamanhoNome > 0) {
                nomeJogador[tamanhoNome - 1] = '\0';
                tamanhoNome--;
            } else if (key == KEY_ENTER && tamanhoNome > 0) {
                return;
            } else if ((key >= 32 && key <= 126) && tamanhoNome < 3) {
                nomeJogador[tamanhoNome] = (char)key;
                tamanhoNome++;
                nomeJogador[tamanhoNome] = '\0';
            }
            key = GetKeyPressed();
        }
        EndDrawing();
    }
}

int VerificaTop(Jogo* j) {
    j->arquivo = fopen("../doc/ranking.txt", "r");
    if (j->arquivo == NULL) return 1;
    int pontuacoes[10];
    char nomes[10][50];
    for (int i = 0; i < 10; i++) {
        if (fscanf(j->arquivo, "%d|%49s", &pontuacoes[i], nomes[i]) != 2) break;
    }
    fclose(j->arquivo);
    j->arquivo = NULL;
    for (int i = 0; i < 10; i++) {
        if (j->pontuacao > pontuacoes[i]) return 1;
    }
    return 0;
}

void SalvarRanking(Jogo* j) {
    j->arquivo = fopen("../doc/ranking.txt", "r+");

    if (j->arquivo == NULL) {
        j->arquivo = fopen("../doc/ranking.txt", "w");
        if (j->arquivo != NULL) {
            fprintf(j->arquivo, "%d|%s\n", j->pontuacao, j->nomeJogador);
            fclose(j->arquivo);
            j->arquivo = NULL;
        }
    } else {
        int pontuacoes[10];
        char nomes[10][50];
        int totalPontuacoes = 0;

        while (fscanf(j->arquivo, "%d|%49s", &pontuacoes[totalPontuacoes], nomes[totalPontuacoes]) == 2) {
            totalPontuacoes++;
        }
        fclose(j->arquivo);
        j->arquivo = NULL;

        if (totalPontuacoes < 10) {
            pontuacoes[totalPontuacoes] = j->pontuacao;
            strcpy(nomes[totalPontuacoes], j->nomeJogador);
            totalPontuacoes++;
        } else if (j->pontuacao > pontuacoes[9]) {
            pontuacoes[9] = j->pontuacao;
            strcpy(nomes[9], j->nomeJogador);
        }

        for (int i = 0; i < totalPontuacoes; i++) {
            for (int j = i + 1; j < totalPontuacoes; j++) {
                if (pontuacoes[i] < pontuacoes[j]) {
                    int temp = pontuacoes[i];
                    pontuacoes[i] = pontuacoes[j];
                    pontuacoes[j] = temp;
                    char tempNome[50];
                    strcpy(tempNome, nomes[i]);
                    strcpy(nomes[i], nomes[j]);
                    strcpy(nomes[j], tempNome);
                }
            }
        }

        j->arquivo = fopen("../doc/ranking.txt", "w");
        if (j->arquivo != NULL) {
            for (int i = 0; i < (totalPontuacoes < 10 ? totalPontuacoes : 10); i++) {
                fprintf(j->arquivo, "%d|%s\n", pontuacoes[i], nomes[i]);
            }
            fclose(j->arquivo);
            j->arquivo = NULL;
        }
    }
}

/* ---------------------------- */
/*    FUNÇÕES AUXILIARES        */
/* ---------------------------- */
int FinalDeJogo(Jogo* j){
if (j->heroi.vidas <= 0 || IsKeyDown(KEY_F2)) {
    if (VerificaTop(j)) {
        SalvarRanking(j);
    }
    return TelaDerrota(j);
}
    return 0;
}

int DoubleEquals(double a, double b) {
    return fabs(a - b) < EPSILON;
}
