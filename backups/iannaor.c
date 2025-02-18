#include "raylib.h"
    #include <stdlib.h>
    #include <stdio.h>
    #include <time.h>
    #include <string.h>
    #include <math.h>

    #define LARGURA_JANELA 800
    #define ALTURA_JANELA 600
    #define STD_SIZE_X 32
    #define STD_SIZE_Y 32
    #define LARGURA_BALA 10
    #define ALTURA_BALA 15

    typedef struct Bala{
        Rectangle pos;
        Color color;
        int ativa;
        int tempo;
        int velocidade;
        Sound tiro;
    }Bala;

    typedef struct Barreira {
        Rectangle pos;
        int ativa;
        int vida;
    } Barreira;


    typedef struct Nave{
        Rectangle pos;
        Color color;
        Bala bala;
        int velocidade;
        int direcao;
    }Nave;

    typedef struct Heroi{
        Rectangle pos;
        Color color;
        Bala bala;
        int velocidade;
        int direcao;
        int vidas;
    }Heroi;

    typedef struct Bordas{
        Rectangle pos;
    } Bordas;

    typedef struct Assets{
        Texture2D naveVerde;
        Texture2D heroiPrata;
        Texture2D telaFinal;
        Sound tiro;
    }Assets;

    typedef struct Jogo{
        Nave nave;
        Heroi heroi;
        Bordas bordas[4];
        Assets assets;
        Barreira barreiras[3];
        int alturaJanela;
        int larguraJanela;
        int tempoAnimacao;
    }Jogo;

    void IniciaJogo(Jogo *j);
    void IniciaNaves(Jogo *j);
    void IniciaBarreiras(Jogo *j);
    void AtualizaJogo(Jogo *j);
    void AtualizaBarreiras (Jogo *j);
    void DesenhaJogo(Jogo *j);
    void DesenhaBarreiras(Jogo *j);
    void AtualizaFrameDesenho(Jogo *j);
    void AtualizaNavePos(Jogo *j);
    void AtualizaHeroiPos(Jogo *j);
    void DesenhaDerrota (Jogo *j);
    void DesenhaNaves(Jogo *j);
    void DesenhaHeroi(Jogo *j);
    void DesenhaVidas(Jogo *j);
    void DesenhaVitoria(Jogo* j);
    void ColisaoBordas(Jogo *j);
    void DesenhaBordas(Jogo *j);
    int ColisaoBalas(Jogo *j);
    int ColisaoBalasHeroi(Jogo *j);
    void DesenhaBordas(Jogo *j);
    void AtiraBalas(Jogo *j);
    void AtiraBalasHeroi(Jogo *j);
    void CarregaImagens(Jogo *j);
    void DescarregaImagens(Jogo *j);
    void DesenhaBalas(Jogo *j);
    void DesenhaBalasHeroi(Jogo *j);
    int FinalDeJogo(Jogo* j);



    int main(){
        InitAudioDevice();

        Jogo jogo;

        jogo.alturaJanela = ALTURA_JANELA;
        jogo.larguraJanela = LARGURA_JANELA;

        InitWindow(jogo.larguraJanela, jogo.alturaJanela, "Space Invaders");
        SetTargetFPS(60);
        IniciaJogo(&jogo);
        CarregaImagens(&jogo);
        Music musicaJogo = LoadMusicStream("assets/musica.mp3");
        PlayMusicStream(musicaJogo);

        while(TelaInicial());

        while(!WindowShouldClose() && !FinalDeJogo(&jogo)){
            UpdateMusicStream(musicaJogo);
            AtualizaFrameDesenho(&jogo);
        }

        UnloadMusicStream(musicaJogo);
        DescarregaImagens(&jogo);
        CloseWindow();
        return 0;
    }

    void IniciaJogo(Jogo *j){
        j->tempoAnimacao = GetTime();

        j->heroi.pos = (Rectangle) {LARGURA_JANELA/2 - STD_SIZE_X/2, ALTURA_JANELA - STD_SIZE_Y -10, STD_SIZE_X, STD_SIZE_Y};
        j->heroi.color = PURPLE;
        j->heroi.velocidade = 3;
        j->heroi.bala.ativa = 0;
        j->heroi.bala.tempo = GetTime();
        j->heroi.bala.velocidade = 15;
        j->heroi.bala.tiro = LoadSound("assets/shoot.wav");
        j->heroi.vidas = 3;

        j->nave.pos = (Rectangle) {0, 15, STD_SIZE_X, STD_SIZE_Y};
        j->nave.color = RED;
        j->nave.velocidade = 3;
        /*direcao = 1 faz nave mover para direita, direcao = 0 faz mover para esquerda*/
        j->nave.direcao = 1;
        j->nave.bala.ativa = 0;
        j->nave.bala.tempo = GetTime();
        j->nave.bala.velocidade = 5;
        j->nave.bala.tiro = LoadSound("assets/shoot.wav");

        //borda encima
        j->bordas[0].pos = (Rectangle){0, 0, LARGURA_JANELA, 10};
        //borda embaixo
        j->bordas[1].pos = (Rectangle){0, ALTURA_JANELA-10, LARGURA_JANELA, 10};
        //borda esquerda
        j->bordas[2].pos = (Rectangle){0, 0, 10, ALTURA_JANELA};
        //borda direita
        j->bordas[3].pos = (Rectangle){LARGURA_JANELA-10, 0, 10, ALTURA_JANELA};
        IniciaBarreiras(j);
    }

    void IniciaNaves(Jogo *j){

    }

    void AtualizaJogo(Jogo *j){
        AtualizaNavePos(j);
        AtualizaHeroiPos(j);
        AtiraBalas(j);
        AtiraBalasHeroi(j);
        AtualizaBarreiras(j);
    }

    void DesenhaJogo(Jogo *j){
        BeginDrawing();
        ClearBackground(BLACK);
        DesenhaNaves(j);
        DesenhaHeroi(j);
        DesenhaBordas(j);
        DesenhaVidas(j);
        DesenhaBarreiras(j);
        EndDrawing();
    }

    int FinalDeJogo(Jogo* j){
        if (CheckCollisionRecs(j->nave.pos, j->heroi.bala.pos)) {
                DesenhaVitoria(j);
                return 1;
                }
        if (j->heroi.vidas <= 0) {
            DesenhaDerrota(j);
            return 1;
        }
        return 0;
    }
    void DesenhaVitoria(Jogo* j){
        j->assets.telaFinal = LoadTexture("assets/YouWin.png");
            while(IsKeyUp(KEY_ESCAPE) && !WindowShouldClose()){
                BeginDrawing();
                DrawTexture(j->assets.telaFinal, 0, 0, WHITE);
                EndDrawing();
            }
    }

    void DesenhaDerrota(Jogo* j){
        j->assets.telaFinal = LoadTexture("assets/YouLose.png");
            while(IsKeyUp(KEY_ESCAPE) && !WindowShouldClose()){
                BeginDrawing();
                DrawTexture(j->assets.telaFinal, 0, 0, WHITE);
                EndDrawing();
            }
    }

    void AtualizaFrameDesenho(Jogo *j){
        AtualizaJogo(j);
        DesenhaJogo(j);
    }

    void ColisaoBordas(Jogo *j){
        if(CheckCollisionRecs(j->nave.pos, j->bordas[2].pos)){
            j->nave.direcao = 1;
        }else if(CheckCollisionRecs(j->nave.pos, j->bordas[3].pos)){
            j->nave.direcao = 0;
            }
        }


    void AtualizaNavePos(Jogo *j){
        ColisaoBordas(j);
        if(j->nave.direcao == 1){
            j->nave.pos.x += j->nave.velocidade;
        }else{
            j->nave.pos.x -= j->nave.velocidade;
        }
    }

    void AtualizaHeroiPos(Jogo *j){
        if(IsKeyDown(KEY_RIGHT) && j->heroi.pos.x < LARGURA_JANELA-STD_SIZE_X - 10){
            j->heroi.pos.x += j->heroi.velocidade;
        }else if(IsKeyDown(KEY_LEFT) && j->heroi.pos.x > 10){
            j->heroi.pos.x -= j->heroi.velocidade;
        }
    }

    void CarregaImagens(Jogo *j){
        j->assets.naveVerde = LoadTexture("assets/GreenAnimation.png");
        j->assets.heroiPrata = LoadTexture("assets/SilverStatic.png");
    }

    void DescarregaImagens(Jogo *j){
        UnloadTexture(j->assets.naveVerde);
        UnloadTexture(j->assets.heroiPrata);
    }

    void DesenhaNaves(Jogo *j){
        Vector2 tamanhoFrame = {32, 32};
        
        static Vector2 frame = {0, 0};
        static float tempoUltimaTroca = 0;
        
        if(GetTime() - tempoUltimaTroca >= 1){
            if(frame.x == 0){
                frame.x = 1;
            }else{
                frame.x = 0;
            }

            tempoUltimaTroca = GetTime();
        }
        Rectangle frameRecNave = {frame.x * tamanhoFrame.x, frame.y*tamanhoFrame.y,
        tamanhoFrame.x, tamanhoFrame.y};
        DrawTexturePro(j->assets.naveVerde, frameRecNave, (Rectangle){j->nave.pos.x, j->nave.pos.y, 32, 32},
        (Vector2){0, 0}, 0.0f, WHITE);
        
    }

    void DesenhaHeroi(Jogo *j){
        DrawTexture(j->assets.heroiPrata, j->heroi.pos.x, j->heroi.pos.y, j->heroi.color);
    }

    void DesenhaVidas(Jogo *j) {
        char textoVidas[20];
        sprintf(textoVidas, "V: %d", j->heroi.vidas);
        DrawText(textoVidas, 10, 10, 20, LIGHTGRAY);
    }

    void DesenhaBordas(Jogo *j){
        for(int i = 0; i < 4; i++){
            DrawRectangleRec(j->bordas[i].pos, LIGHTGRAY);
        }
    }

    void DesenhaBalas(Jogo *j){
        DrawRectangleRec(j->nave.bala.pos, YELLOW);
    }

    void DesenhaBalasHeroi(Jogo *j){
        DrawRectangleRec(j->heroi.bala.pos, RED);
    }

    void AtiraBalas(Jogo *j){
        if(j->nave.bala.ativa == 0 && GetTime()-j->nave.bala.tempo > 3){
            j->nave.bala.pos = (Rectangle){j->nave.pos.x+j->nave.pos.width/2, j->nave.pos.y+j->nave.pos.height/2, 
            LARGURA_BALA, ALTURA_BALA};
            j->nave.bala.ativa = 1;
            j->nave.bala.tempo = GetTime();
            PlaySound(j->nave.bala.tiro);
        }
        else if(ColisaoBalas(j)){
            j->nave.bala.ativa = 0;
        }
        if(j->nave.bala.ativa == 1){
            j->nave.bala.pos.y += j->nave.bala.velocidade;
            DesenhaBalas(j);
        }
    }

    void AtiraBalasHeroi(Jogo *j) {
        if (j->heroi.bala.ativa == 0 && IsKeyDown(KEY_SPACE)) {
            j->heroi.bala.pos = (Rectangle){j->heroi.pos.x + j->heroi.pos.width / 2, j->heroi.pos.y + j->heroi.pos.height / 2, 
            LARGURA_BALA, ALTURA_BALA};
            j->heroi.bala.ativa = 1;
            j->heroi.bala.tempo = GetTime();
            PlaySound(j->heroi.bala.tiro);
        }
        if (j->heroi.bala.ativa == 1) {
            j->heroi.bala.pos.y -= j->heroi.bala.velocidade;
            DesenhaBalasHeroi(j);
            for (int i = 0; i < 3; i++) {
                if (CheckCollisionRecs(j->heroi.bala.pos, j->barreiras[i].pos)) {
                    j->heroi.bala.ativa = 0;
                    j->heroi.bala.pos.x = 999;
                    j->heroi.bala.pos.y = 999;
                    break;
                }
            }
            if (j->heroi.bala.pos.y < 0) {
                j->heroi.bala.ativa = 0;
                j->heroi.bala.pos.x = 999;
                j->heroi.bala.pos.y = 999;
            }
        }
    }
    int ColisaoBalas(Jogo *j) {
        if (CheckCollisionRecs(j->nave.bala.pos, j->heroi.pos)) {
            j->nave.bala.ativa = 0;
            j->heroi.vidas--;
            j->nave.bala.pos.x = 999;
            j->nave.bala.pos.y = 999;
            return 1;
        }
        if (CheckCollisionRecs(j->nave.bala.pos, j->bordas[1].pos)) {
            j->nave.bala.pos.x = 999;
            j->nave.bala.pos.y = 999;
            j->nave.bala.ativa = 0;
            return 1;
        }
            for (int i = 0; i < 3; i++) {
                if (j->barreiras[i].ativa && CheckCollisionRecs(j->nave.bala.pos, j->barreiras[i].pos)) {
                    j->nave.bala.ativa = 0;
                    j->nave.bala.pos.x = 999; 
                    j->nave.bala.pos.y = 999;
                return 1;
            }
        }

        return 0;
    }


    int ColisaoBalasHeroi(Jogo *j) {
        if (CheckCollisionRecs(j->nave.pos, j->heroi.bala.pos)) {
            j->heroi.bala.ativa = 0;
            j->heroi.bala.pos.x = 999;
            j->heroi.bala.pos.y = 999;
            return 1;
        }
        if (CheckCollisionRecs(j->heroi.bala.pos, j->bordas[0].pos)) {
            j->heroi.bala.ativa = 0;
            j->heroi.bala.pos.x = 999;
            j->heroi.bala.pos.y = 999;
            return 1;
        }   
        
        return 0;
    }
    void IniciaBarreiras(Jogo *j) {
        j->barreiras[0].pos = (Rectangle) {150 - 75 / 2, ALTURA_JANELA / 2 + 222, 75, 15};
        j->barreiras[1].pos = (Rectangle) {400 - 75 / 2, ALTURA_JANELA / 2 + 222, 75, 15};
        j->barreiras[2].pos = (Rectangle) {650 - 75 / 2, ALTURA_JANELA / 2 + 222, 75, 15};
        for (int i = 0; i < 3; i++) {
            j->barreiras[i].ativa = 1;
            j->barreiras[i].vida = 3;
        }
    }


    void DesenhaBarreiras(Jogo *j) {
        for (int i = 0; i < 3; i++) {
            if (j->barreiras[i].ativa) {
                DrawRectangleRec(j->barreiras[i].pos, BLUE);
            }
        }
    }

    void AtualizaBarreiras (Jogo *j) {
        for (int i = 0; i < 3; i++) {
            if (j->barreiras[i].ativa && CheckCollisionRecs(j->nave.bala.pos, j->barreiras[i].pos) && j->nave.bala.ativa) {
                j->nave.bala.ativa = 0;
                j->nave.bala.pos.x = 999;
                j->nave.bala.pos.y = 999;
                j->barreiras[i].vida--;
                if (j->barreiras[i].vida == 0) {
                    j->barreiras[i].ativa = 0;
                    j->nave.bala.ativa = 0;

                }
            }
        }
    }