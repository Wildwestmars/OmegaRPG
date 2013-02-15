/* omega copyright (c) 1987,1988,1989 by Laurence Raphael Brothers */
/* mmove.c */
/* monster move functions */

#include "glob.h"

/* like m_normal_move, but can open doors */
void m_smart_move(Monster* m)
{
    m_simple_move(m);
}

/* not very smart, but not altogether stupid movement */
void m_normal_move(Monster* m)
{
    m_simple_move(m);
}

/* used by both m_normal_move and m_smart_move */
void m_simple_move(Monster* m)
{
    int dx = sign(Player.x - m->x);
    int dy = sign(Player.y - m->y);

    erase_monster(m);
    if (m->hp < Monsters[m->id].hp/4) {
        dx = - dx;
        dy = - dy;
        m->movef = M_MOVE_SCAREDY;
        if (m->uniqueness == COMMON) {
            strcpy(Str2,"The ");
            strcat(Str2,m->name);
        }
        else strcpy(Str2,m->name);
        if (m->possessions != NULL) {
            strcat(Str2," drops its treasure and flees!");
            m->m_dropstuff();
        }
        else strcat(Str2," flees!");
        mprint(Str2);
        m->speed = min(2,m->speed-1);
    }
    if ((! m_statusp(m,HOSTILE) && !m_statusp(m, NEEDY)) ||
            (Player.status[INVISIBLE] > 0)) m_random_move(m);
    else {
        if (m_unblocked(m,m->x+dx,m->y+dy))
            m->movemonster(m->x+dx,m->y+dy);
        else if (dx == 0) {
            if (m_unblocked(m,m->x+1,m->y+dy))
                m->movemonster(m->x+1,m->y+dy);
            else if (m_unblocked(m,m->x-1,m->y+dy))
                m->movemonster(m->x-1,m->y+dy);
        }

        else if (dy == 0) {
            if (m_unblocked(m,m->x+dx,m->y+1))
                m->movemonster(m->x+dx,m->y+1);
            else if (m_unblocked(m,m->x+dx,m->y-1))
                m->movemonster(m->x+dx,m->y-1);
        }

        else {
            if (m_unblocked(m,m->x+dx,m->y))
                m->movemonster(m->x+dx,m->y);
            else if (m_unblocked(m,m->x,m->y+dy))
                m->movemonster(m->x,m->y+dy);
        }
    }
}


void m_move_animal(Monster* m)
{
    if (m_statusp(m,HOSTILE))
        m_normal_move(m);
    else m_scaredy_move(m);
}


/* same as simple move except run in opposite direction */
void m_scaredy_move(Monster* m)
{
    int dx = -sign(Player.x - m->x);
    int dy = -sign(Player.y - m->y);
    erase_monster(m);
    if (Player.status[INVISIBLE]) m_random_move(m);
    else {
        if (m_unblocked(m,m->x+dx,m->y+dy))
            m->movemonster(m->x+dx,m->y+dy);
        else if (dx == 0) {
            if (m_unblocked(m,m->x+1,m->y+dy))
                m->movemonster(m->x+1,m->y+dy);
            else if (m_unblocked(m,m->x-1,m->y+dy))
                m->movemonster(m->x-1,m->y+dy);
        }

        else if (dy == 0) {
            if (m_unblocked(m,m->x+dx,m->y+1))
                m->movemonster(m->x+dx,m->y+1);
            else if (m_unblocked(m,m->x+dx,m->y-1))
                m->movemonster(m->x+dx,m->y-1);
        }

        else {
            if (m_unblocked(m,m->x+dx,m->y))
                m->movemonster(m->x+dx,m->y);
            else if (m_unblocked(m,m->x,m->y+dy))
                m->movemonster(m->x,m->y+dy);
        }
    }
}


/* for spirits (and earth creatures) who can ignore blockages because
   either they are noncorporeal or they can move through stone */
void m_spirit_move(Monster* m)
{
    int dx = sign(Player.x - m->x);
    int dy = sign(Player.y - m->y);
    erase_monster(m);
    if (m->hp < Monsters[m->id].hp/6) {
        dx = -dx;
        dy = -dy;
    }

    if (Player.status[INVISIBLE] > 0 || !m_unblocked(m, m->x+dx, m->y+dy))
        m_random_move(m);
    else
        m->movemonster(m->x+dx,m->y+dy);
}



/* fluttery dumb movement */
void m_flutter_move(Monster* m)
{
    int trange,range = distance(m->x,m->y,Player.x,Player.y);
    int i,tx,ty,nx=m->x,ny=m->y;
    erase_monster(m);
    if (Player.status[INVISIBLE] > 0) m_random_move(m);
    else {
        for (i=0; i<8; i++) {
            tx = m->x+Dirs[0][i];
            ty = m->y+Dirs[1][i];
            trange = distance(tx,ty,Player.x,Player.y);
            if (m->hp < Monsters[m->id].hp/6) {
                if ((trange > range) && m_unblocked(m,tx,ty)) {
                    range = trange;
                    nx = tx;
                    ny = ty;
                }
            }
            else if ((trange <= range) && m_unblocked(m,tx,ty)) {
                range = trange;
                nx = tx;
                ny = ty;
            }
        }
        m->movemonster(nx,ny);
    }
}


void m_follow_move(Monster* m)
{
    if (! m_statusp(m,HOSTILE))
        m_normal_move(m);
    else m_scaredy_move(m);
}



/* allows monsters to fall into pools, revealed traps, etc */
void m_confused_move(Monster* m)
{
    int i,nx,ny,done=false;
    erase_monster(m);
    for (i=0; ((i<8)&&(! done)); i++) {
        nx = m->x+random_range(3)-1;
        ny = m->y+random_range(3)-1;
        if (unblocked(nx,ny) &&
                ((nx != Player.x) ||
                 (ny != Player.y))) {
            done = true;
            m->movemonster(nx,ny);
        }
    }
}

void m_random_move(Monster* m)
{
    int i,nx,ny,done=false;
    erase_monster(m);
    for (i=0; ((i<8)&&(! done)); i++) {
        nx = m->x+random_range(3)-1;
        ny = m->y+random_range(3)-1;
        if (m_unblocked(m,nx,ny) &&
                ((nx != Player.x) ||
                 (ny != Player.y))) {
            done = true;
            m->movemonster(nx,ny);
        }
    }
}

/* monster removed from play */
void m_vanish(Monster* m)
{
    if (m->uniqueness == COMMON) {
        strcpy(Str2,"The ");
        strcat(Str2,m->name);
    }
    else strcpy(Str2,m->name);
    strcat(Str2," vanishes in the twinkling of an eye!");
    mprint(Str2);
    m->m_remove();/* signals "death" -- no credit to player, though */
}

/* monster still in play */
void m_teleport(Monster* m)
{
    erase_monster(m);
    if (m_statusp(m,AWAKE)) {
        Level->site[m->x][m->y].creature = NULL;
        putspot(m->x,m->y,getspot(m->x,m->y,false));
        findspace(&(m->x),&(m->y),-1);
        Level->site[m->x][m->y].creature = m;
    }
}

void m_move_leash(Monster* m)
{
    m_simple_move(m);
    if (m->aux1 == 0) {
        m->aux1 = m->x;
        m->aux2 = m->y;
    }
    else if (distance(m->x,m->y,m->aux1,m->aux2) > 5) {
        if (Level->site[m->aux1][m->aux2].creature != NULL) {
            if (los_p(Player.x,Player.y,m->aux1,m->aux2)) {
                /* some other monster is where the chain starts */
                if (Level->site[m->aux1][m->aux2].creature->uniqueness == COMMON) {
                    strcpy(Str1, "The ");
                    strcat(Str1, Level->site[m->aux1][m->aux2].creature->name);
                }
                else
                    strcpy(Str1, Level->site[m->aux1][m->aux2].creature->name);
                strcat(Str1, " releases the dog's chain!");
                mprint(Str1);
            }
            m->movef = M_MOVE_NORMAL;
            /* otherwise, we'd lose either the dog or the other monster. */
        }
        else if (los_p(Player.x,Player.y,m->x,m->y)) {
            mprint("You see the dog jerked back by its chain!");
            plotspot(m->x, m->y, false);
        }
        else mprint("You hear a strangled sort of yelp!");
        Level->site[m->x][m->y].creature = NULL;
        m->x = m->aux1;
        m->y = m->aux2;
        Level->site[m->x][m->y].creature = m;
    }
}