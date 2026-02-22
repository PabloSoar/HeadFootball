// Bibliotecas
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

//Constantes do Jogo
const float FPS = 90;
const int SCREEN_W = 1400;
const int SCREEN_H = 768;
const int ALTURA_CHAO = 718;
const int BOLA_TAMANHO = 70;
const int JOGADOR_LARGURA = 160;
const int JOGADOR_ALTURA = 200;
const float GRAVIDADE = 0.1;
const float FORCA_PULO = -7;
const int GOLEIRA_ALTURA = 400;
const int GOLEIRA_LARGURA = 20;
const int MAX_GOLS = 5;


//Variáveis Globais
float bola_pos_x, bola_pos_y, bola_vx, bola_vy;
float jogador1_x, jogador1_y, jogador1_vx, jogador1_vy;
float jogador2_x, jogador2_y, jogador2_vx, jogador2_vy;
bool jogador1_no_chao = true, jogador2_no_chao = true;
bool redraw = true;
int gols_jogador1 = 0, gols_jogador2 = 0;
bool jogo_encerrado = false;
ALLEGRO_FONT *fonte = NULL;
ALLEGRO_BITMAP *background = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_SAMPLE *som_gol = NULL;
ALLEGRO_SAMPLE *som_cabecada = NULL;
ALLEGRO_SAMPLE *som_chute = NULL;
ALLEGRO_AUDIO_STREAM *musica_fundo = NULL;


//"Pré" Funções
void inicializar_jogo();
void atualizar_jogo();
void desenhar_tela(ALLEGRO_BITMAP *bola, ALLEGRO_BITMAP *jogador1, ALLEGRO_BITMAP *jogador2);
void verificar_colisoes();
void verificar_gols();
void desenhar_mensagem_vitoria(int jogador);

//main
int main(int argc, char **argv)
{
    //Variáveis Allegro
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_BITMAP *bola = NULL;
    ALLEGRO_BITMAP *jogador1 = NULL;
    ALLEGRO_BITMAP *jogador2 = NULL;

    //Inicializar Allegro
    if (!al_init())
    {
        fprintf(stderr, "Falha ao inicializar Allegro!\n");
        return -1;
    }

    al_install_keyboard();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();

    // Display e Timer
    display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display)
    {
        fprintf(stderr, "Falha ao criar display!\n");
        return -1;
    }

    timer = al_create_timer(1.0 / FPS);
    if (!timer)
    {
        fprintf(stderr, "Falha ao criar timer!\n");
        al_destroy_display(display);
        return -1;
    }

    event_queue = al_create_event_queue();
    if (!event_queue)
    {
        fprintf(stderr, "Falha ao criar fila de eventos!\n");
        al_destroy_timer(timer);
        al_destroy_display(display);
        return -1;
    }

    //Inicializa o suporte a fontes TrueType
    if (!al_init_ttf_addon())
    {
        fprintf(stderr, "Falha ao inicializar o suporte a fontes TrueType!\n");
        return -1;
    }

    //Inicialização sistema de áudio
    if (!al_install_audio())
    {
        printf("Erro ao inicializar o sistema de áudio!\n");
        return -1;
    }
    if (!al_init_acodec_addon())
    {
        printf("Erro ao inicializar os codecs de áudio!\n");
        return -1;
    }
    if (!al_reserve_samples(3))
    {
        printf("Erro ao reservar canais de áudio!\n");
        return -1;
    }

    // Carregar Recursos
    bola = al_load_bitmap("imagens/bola.png");
    if (!bola)
    {
        fprintf(stderr, "Falha ao carregar sprite da bola!\n");
        return -1;
    }

    jogador1 = al_load_bitmap("imagens/jogador1.png");
    if (!jogador1)
    {
        fprintf(stderr, "Falha ao carregar sprite do jogador 1!\n");
        return -1;
    }

    jogador2 = al_load_bitmap("imagens/jogador2.png");
    if (!jogador2)
    {
        fprintf(stderr, "Falha ao carregar sprite do jogador 2!\n");
        return -1;
    }

    background = al_load_bitmap("imagens/background.png");
    if (!background)
    {
        fprintf(stderr, "Falha ao carregar o sprite do background!\n");
        return -1;
    }

    //Carregar música de fundo
    musica_fundo = al_load_audio_stream("sons/musica_fundo.ogg", 4, 1024);
    if (!musica_fundo)
    {
        printf("Erro ao carregar a música de fundo!\n");
    }

    //Carregar efeitos sonoros
    som_gol = al_load_sample("sons/som_gol.ogg");
    som_cabecada = al_load_sample("sons/som_cabecada.ogg");
    som_chute = al_load_sample("sons/som_chute.ogg");

    if (!som_gol || !som_cabecada || !som_chute)
    {
        printf("Erro ao carregar efeitos sonoros!\n");
    }

    // Carregar a fonte
    fonte = al_load_ttf_font("fontes/placar.ttf", 50, 0);
    if (!fonte)
    {
        fprintf(stderr, "Falha ao carregar a fonte!\n");
        return -1;
    }


    // Registro de Eventos
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_start_timer(timer);

    inicializar_jogo();

    // Game Loop
    while (1)
    {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER)
        {
            if (!jogo_encerrado)
            {
                atualizar_jogo();
                verificar_gols();
            }
            redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN || ev.type == ALLEGRO_EVENT_KEY_UP)
        {
            if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            {
                break;
            }

            if (!jogo_encerrado)
            {
                switch (ev.keyboard.keycode)
                {
                    case ALLEGRO_KEY_W:
                        if (jogador1_no_chao)
                        {
                            jogador1_vy = FORCA_PULO;
                            jogador1_no_chao = false;
                        }
                        break;
                    case ALLEGRO_KEY_UP:
                        if (jogador2_no_chao)
                        {
                            jogador2_vy = FORCA_PULO;
                            jogador2_no_chao = false;
                        }
                        break;
                    case ALLEGRO_KEY_A:
                        jogador1_vx = -4;
                        break;
                    case ALLEGRO_KEY_D:
                        jogador1_vx = 4;
                        break;
                    case ALLEGRO_KEY_LEFT:
                        jogador2_vx = -4;
                        break;
                    case ALLEGRO_KEY_RIGHT:
                        jogador2_vx = 4;
                        break;
                }

                if (ev.type == ALLEGRO_EVENT_KEY_UP)
                {
                    switch (ev.keyboard.keycode)
                    {
                        case ALLEGRO_KEY_A:
                        case ALLEGRO_KEY_D:
                            jogador1_vx = 0;
                            break;
                        case ALLEGRO_KEY_LEFT:
                        case ALLEGRO_KEY_RIGHT:
                            jogador2_vx = 0;
                            break;
                    }
                }
            }
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
        {
            break;
        }

        if (redraw && al_is_event_queue_empty(event_queue))
        {
            redraw = false;
            desenhar_tela(bola, jogador1, jogador2);
        }
    }

    finalizar_jogo(bola, jogador1, jogador2, timer, display, event_queue);


    return 0;
}

// Funções
void inicializar_jogo()
{
    tocar_musica();

    //Posição e velocidade da bola
    bola_pos_x = SCREEN_W / 2.0 - BOLA_TAMANHO / 2.0;
    bola_pos_y = ALTURA_CHAO - BOLA_TAMANHO;
    bola_vx = 4;
    bola_vy = -4;

    //Posição e velocidade do jogador 1
    jogador1_x = 100;
    jogador1_y = ALTURA_CHAO - JOGADOR_ALTURA;
    jogador1_vx = 0;
    jogador1_vy = 0;

    //Posição e velocidade do jogador 2
    jogador2_x = SCREEN_W - 100 - JOGADOR_LARGURA;
    jogador2_y = ALTURA_CHAO - JOGADOR_ALTURA;
    jogador2_vx = 0;
    jogador2_vy = 0;

    //Placar 0x0
    gols_jogador1 = 0;
    gols_jogador2 = 0;

    jogo_encerrado = false;
}

void atualizar_jogo()
{
    //Gravidade nos Jogadores
    jogador1_vy += GRAVIDADE;
    jogador2_vy += GRAVIDADE;

    //Att posições
    jogador1_x += jogador1_vx;
    jogador1_y += jogador1_vy;
    jogador2_x += jogador2_vx;
    jogador2_y += jogador2_vy;

    //Colisão Jogador X Chão
    if (jogador1_y > ALTURA_CHAO - JOGADOR_ALTURA)
    {
        jogador1_y = ALTURA_CHAO - JOGADOR_ALTURA;
        jogador1_vy = 0;
        jogador1_no_chao = true;
    }

    if (jogador2_y > ALTURA_CHAO - JOGADOR_ALTURA)
    {
        jogador2_y = ALTURA_CHAO - JOGADOR_ALTURA;
        jogador2_vy = 0;
        jogador2_no_chao = true;
    }

    //Jogador 1 - Borda horizontal
    if (jogador1_x < 0) {
        jogador1_x = 0;
    }

    //Jogador 2 - Borda horizontal
        if (jogador2_x + JOGADOR_LARGURA > SCREEN_W)
    {
        jogador2_x = SCREEN_W - JOGADOR_LARGURA;
    }

    //Impede que a bola saia horizontalmente
    if (bola_pos_x < 0)
    {
        bola_pos_x = 0;
        bola_vx = -bola_vx;
    }
    else if (bola_pos_x + BOLA_TAMANHO > SCREEN_W)
    {
        bola_pos_x = SCREEN_W - BOLA_TAMANHO;
        bola_vx = -bola_vx;
    }

    // Impede que a bola saia verticalmente
    if (bola_pos_y < 0)
    {
        bola_pos_y = 0;
        bola_vy = -bola_vy;
    }
    else if (bola_pos_y + BOLA_TAMANHO > ALTURA_CHAO)
    {
        bola_pos_y = ALTURA_CHAO - BOLA_TAMANHO;
        bola_vy = -bola_vy;
    }

    //Movimento da Bola
    bola_pos_x += bola_vx;
    bola_pos_y += bola_vy;

    bola_vy += GRAVIDADE;

    verificar_colisoes();

    //Velocidade máxima da bola
    const float VELOCIDADE_MAXIMA = 10;
    if (bola_vx > VELOCIDADE_MAXIMA) bola_vx = VELOCIDADE_MAXIMA;
    if (bola_vx < -VELOCIDADE_MAXIMA) bola_vx = -VELOCIDADE_MAXIMA;
    if (bola_vy > VELOCIDADE_MAXIMA) bola_vy = VELOCIDADE_MAXIMA;
    if (bola_vy < -VELOCIDADE_MAXIMA) bola_vy = -VELOCIDADE_MAXIMA;

}

void verificar_gols()
{
    if (bola_pos_x < GOLEIRA_LARGURA &&
        bola_pos_y + BOLA_TAMANHO > ALTURA_CHAO - GOLEIRA_ALTURA)
    {
        gols_jogador2++;
        if (gols_jogador2 >= MAX_GOLS)
        {
            jogo_encerrado = true;
            al_play_sample(som_gol, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            desenhar_mensagem_vitoria(2);
            inicializar_jogo();
        }
        else
        {
            al_play_sample(som_gol, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            desenhar_mensagem_gol(2);
            reiniciar_posicoes(-1);
        }
    }
    else if (bola_pos_x + BOLA_TAMANHO > SCREEN_W - GOLEIRA_LARGURA &&
             bola_pos_y + BOLA_TAMANHO > ALTURA_CHAO - GOLEIRA_ALTURA)
    {
        gols_jogador1++;
        if (gols_jogador1 >= MAX_GOLS)
        {
            jogo_encerrado = true;
            al_play_sample(som_gol, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            desenhar_mensagem_vitoria(1);
            inicializar_jogo();
        }
        else
        {
            al_play_sample(som_gol, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            desenhar_mensagem_gol(1);
            reiniciar_posicoes(1);
        }
    }
}



void verificar_colisoes()
{
    //Colisão Bola X Chão
    if (bola_pos_y > ALTURA_CHAO - BOLA_TAMANHO) {
        bola_pos_y = ALTURA_CHAO - BOLA_TAMANHO;

        //Perda de energia controlada
        if (fabs(bola_vy) < 1) {
            bola_vy = 0; //Para evitar "quicar" infinitamente
        } else {
            bola_vy *= -0.8; //Rebote com perda de velocidade
        }
    }

    //Colisão Bola X Jogador 1
    if (bola_pos_x < jogador1_x + JOGADOR_LARGURA &&
        bola_pos_x + BOLA_TAMANHO > jogador1_x &&
        bola_pos_y < jogador1_y + JOGADOR_ALTURA &&
        bola_pos_y + BOLA_TAMANHO > jogador1_y)
    {

        //Colisão pela parte superior do jogador 1 (Cabeçada)
        if (bola_pos_y + BOLA_TAMANHO <= jogador1_y + 10)
        {
            bola_vy = -fabs(bola_vy);
            bola_vx = fabs(bola_vx);
            al_play_sample(som_cabecada, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            bola_pos_y = jogador1_y - BOLA_TAMANHO; //Evita "clipping"
        }

        //Colisão pelo lado do jogador 1 (Chute)
        else if (bola_pos_x + BOLA_TAMANHO / 2 < jogador1_x + JOGADOR_LARGURA / 2)
        {
            bola_pos_x = jogador1_x - BOLA_TAMANHO; //Colisão pela esquerda
            bola_vx = -fabs(bola_vx);
            al_play_sample(som_chute, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        }
        else
        {
            bola_pos_x = jogador1_x + JOGADOR_LARGURA; //Colisão pela direita
            bola_vx = fabs(bola_vx) * 1.5;
            al_play_sample(som_chute, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        }

        bola_vy += jogador1_vy * 3; //Impacto vertical com movimento do jogador
    }

    //Colisão Bola X Jogador 2
    if (bola_pos_x < jogador2_x + JOGADOR_LARGURA &&
        bola_pos_x + BOLA_TAMANHO > jogador2_x &&
        bola_pos_y < jogador2_y + JOGADOR_ALTURA &&
        bola_pos_y + BOLA_TAMANHO > jogador2_y)
    {

        //Colisão pela parte superior do jogador 2 (Cabeçada)
        if (bola_pos_y + BOLA_TAMANHO <= jogador2_y + 10)
        {
            bola_vy = -fabs(bola_vy);
            bola_vx = -fabs(bola_vx);
            al_play_sample(som_cabecada, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            bola_pos_y = jogador2_y - BOLA_TAMANHO; //Evita "clipping"
        }

        //Colisão pelo lado do jogador 2 (Chute)
        else if (bola_pos_x + BOLA_TAMANHO / 2 < jogador2_x + JOGADOR_LARGURA / 2)
        {
            bola_pos_x = jogador2_x - BOLA_TAMANHO; //Colisão pela esquerda
            bola_vx = -fabs(bola_vx) * 1.5;
            al_play_sample(som_chute, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        }
        else
        {
            bola_pos_x = jogador2_x + JOGADOR_LARGURA; //Colisão pela direita
            bola_vx = fabs(bola_vx);
            al_play_sample(som_chute, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        }

        bola_vy += jogador2_vy * 3;// Impacto vertical com movimento do jogador
    }
}

void reiniciar_posicoes(ultimo_gol)
{
    //Posição e velocidade da Bola
    bola_pos_x = SCREEN_W / 2.0 - BOLA_TAMANHO / 2.0;
    bola_pos_y = ALTURA_CHAO - BOLA_TAMANHO;
    bola_vx = 4 * (ultimo_gol);
    bola_vy = -4;

    //Posição e velocidade do jogador 1
    jogador1_x = 100;
    jogador1_y = ALTURA_CHAO - JOGADOR_ALTURA;
    jogador1_vx = 0;
    jogador1_vy = 0;

    //Posição e velocidade do jogador 2
    jogador2_x = SCREEN_W - 100 - JOGADOR_LARGURA;
    jogador2_y = ALTURA_CHAO - JOGADOR_ALTURA;
    jogador2_vx = 0;
    jogador2_vy = 0;
}


void desenhar_tela(ALLEGRO_BITMAP *bola, ALLEGRO_BITMAP *jogador1, ALLEGRO_BITMAP *jogador2)
{
    //Background
    al_draw_bitmap(background, 0, 0, 0);

    //Placar
    al_draw_textf(fonte, al_map_rgb(255, 0, 0), SCREEN_W / 2 - 80, 220, 0, "%d", gols_jogador1);
    al_draw_textf(fonte, al_map_rgb(255, 0, 0), SCREEN_W / 2 + 100, 220, 0, "%d", gols_jogador2);

    //Chão, bola e jogadores
    al_draw_filled_rectangle(0, ALTURA_CHAO, SCREEN_W, SCREEN_H, al_map_rgb(200, 200, 200));
    al_draw_bitmap(bola, bola_pos_x, bola_pos_y, 0);
    al_draw_bitmap(jogador1, jogador1_x, jogador1_y, 0);
    al_draw_bitmap(jogador2, jogador2_x, jogador2_y, 0);

    //Desenho das "goleiras"
    al_draw_filled_rectangle(0, ALTURA_CHAO - GOLEIRA_ALTURA, GOLEIRA_LARGURA, ALTURA_CHAO, al_map_rgb(128, 128, 128));
    al_draw_filled_rectangle(SCREEN_W - GOLEIRA_LARGURA, ALTURA_CHAO - GOLEIRA_ALTURA, SCREEN_W, ALTURA_CHAO, al_map_rgb(128, 128, 128));

    al_flip_display();
}


void desenhar_mensagem_vitoria(int jogador)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_draw_textf(fonte, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2,
                  ALLEGRO_ALIGN_CENTER, "Jogador %d Venceu!", jogador);
    al_flip_display();

    double inicio = al_get_time();

    //Loop de "delay"
    while (al_get_time() - inicio < 3.0)
    {
        ALLEGRO_EVENT ev;
        al_get_next_event(event_queue, &ev);
    }
}

void desenhar_mensagem_gol(int jogador)
{
    al_draw_textf(fonte, al_map_rgb(0, 0, 0), SCREEN_W / 2, 600,
                  ALLEGRO_ALIGN_CENTER, "Jogador %d Marcou!", jogador);
    al_flip_display();

    double inicio = al_get_time();

    //Loop de "delay"
    while (al_get_time() - inicio < 2.0)
    {
        ALLEGRO_EVENT ev;
        al_get_next_event(event_queue, &ev);
    }
}

void tocar_musica()
{
    if (musica_fundo)
    {
        al_attach_audio_stream_to_mixer(musica_fundo, al_get_default_mixer());
        al_set_audio_stream_playing(musica_fundo, true);
    }
}

void finalizar_jogo(ALLEGRO_BITMAP *bola, ALLEGRO_BITMAP *jogador1, ALLEGRO_BITMAP *jogador2,
                    ALLEGRO_TIMER *timer, ALLEGRO_DISPLAY *display, ALLEGRO_EVENT_QUEUE *event_queue)
{
    if (bola) al_destroy_bitmap(bola);
    if (jogador1) al_destroy_bitmap(jogador1);
    if (jogador2) al_destroy_bitmap(jogador2);
    if (timer) al_destroy_timer(timer);
    if (display) al_destroy_display(display);
    if (event_queue) al_destroy_event_queue(event_queue);
    if (fonte) al_destroy_font(fonte);
    if (background) al_destroy_bitmap(background);
    if (musica_fundo)
    {
        al_destroy_audio_stream(musica_fundo);
    }
    if (som_gol)
    {
        al_destroy_sample(som_gol);
    }
    if (som_cabecada)
    {
        al_destroy_sample(som_cabecada);
    }
    if (som_chute)
    {
        al_destroy_sample(som_chute);
    }

    al_uninstall_audio();
}

