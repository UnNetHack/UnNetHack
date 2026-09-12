/* Experimental JSON window port. NetHack license applies; see dat/license. */
#include "hack.h"
#include "dlb.h"
#include "func_tab.h"
#include <stdio.h>
#include <stdlib.h>

#define BW 32
#define BM 512
struct entry {
    anything id;
    char *text;
    boolean selectable;
    char accelerator;
    char group_accelerator;
};
struct bwin { int type, n; struct entry items[BM]; char prompt[BUFSZ]; };
static struct bwin wins[BW];
static int glyphs[COLNO][ROWNO], backgrounds[COLNO][ROWNO];
static long request_id;
static void quoted(const char *s) {
    const unsigned char *p=(const unsigned char *)(s?s:"");
    putchar('"');
    for (;*p;p++) { if (*p=='"'||*p=='\\') {putchar('\\');putchar(*p);} else if (*p<32 || *p>=127) printf("\\u%04x",*p); else putchar(*p); }
    putchar('"');
}
static void event(const char *kind,const char *text) { printf("{\"type\":");quoted(kind);printf(",\"text\":");quoted(text);puts("}");fflush(stdout); }
static const char *terrain(int glyph) {
    int c=glyph_to_cmap(glyph);
    if(!glyph_is_cmap(glyph) || c==S_stone) return "unknown";
    if(c>=S_vwall && c<=S_trwall) return "wall";
    if(c==S_vcdoor || c==S_hcdoor) return "door";
    if(c==S_bars) return "bars";
    if(c==S_fountain) return "fountain";
    if(c==S_altar) return "altar";
    if(c==S_upstair || c==S_upladder) return "up";
    if(c==S_dnstair || c==S_dnladder) return "down";
    if(c==S_pool || c==S_water) return "water";
    if(c==S_lava) return "lava";
    if(c==S_tree || c==S_deadtree) return "tree";
    if(c>=S_room && c<=S_litcorr) return "floor";
    if(c==S_ndoor || c==S_vodoor || c==S_hodoor) return "floor";
    return "feature";
}
static const char *object_kind(int glyph) {
    if (glyph_is_body(glyph)) return "corpse";
    if (!glyph_is_object(glyph)) return "";
    return "item";
}
static const char *object_name(int glyph) {
    int o;
    if (glyph_is_body(glyph)) {
        o = glyph - GLYPH_BODY_OFF;
        return (o >= 0 && o < NUMMONS) ? mons[o].mname : "corpse";
    }
    o = glyph_to_obj(glyph);
    return (o >= 0 && o < NUM_OBJECTS) ? OBJ_NAME(objects[o]) : "item";
}
static void frame(void) {
    int x,y,g,b,m,col,terrain_glyph,object_type;glyph_t ch;unsigned special;
    printf("{\"type\":\"frame\",\"turn\":%ld,\"depth\":%d,\"branch\":%d,\"player\":{\"x\":%d,\"z\":%d,\"hp\":%d,\"maxhp\":%d,\"ac\":%d,\"level\":%d,\"weapon\":",moves,depth(&u.uz),u.uz.dnum,u.ux,u.uy,Upolyd?u.mh:u.uhp,Upolyd?u.mhmax:u.uhpmax,u.uac,u.ulevel);
    if (uwep) {
        printf("{\"name\":");quoted(xname(uwep));
        printf(",\"otyp\":%d,\"class\":%d}",uwep->otyp,uwep->oclass);
    } else printf("null");
    printf(",\"helmet\":");
    if (uarmh) {
        printf("{\"name\":");quoted(xname(uarmh));
        printf(",\"otyp\":%d}",uarmh->otyp);
    } else printf("null");
    printf("},\"cells\":[");
    boolean first=TRUE;
    for(y=0;y<ROWNO;y++) for(x=1;x<COLNO;x++) {
        g=glyphs[x][y]; if(g<0)continue;b=backgrounds[x][y];
        if(!first)putchar(',');first=FALSE;
        mapglyph(g,&ch,&col,&special,x,y,0);
        terrain_glyph = glyph_is_cmap(g) ? g : b;
        printf("{\"x\":%d,\"z\":%d,\"glyph\":%d,\"symbol\":%d,\"color\":%d,\"visible\":%s,\"remembered\":%s,\"terrain\":",x,y,g,ch,col,cansee(x,y)?"true":"false",levl[x][y].seenv?"true":"false");quoted(terrain(terrain_glyph));
        printf(",\"kind\":");quoted(glyph_is_pet(g)?"pet":glyph_is_monster(g)?"monster":glyph_is_object(g)?"object":"terrain");
        if (glyph_is_monster(g) && !glyph_is_pet(g)) {
            struct monst *mtmp = m_at(x,y);
            printf(",\"peaceful\":%s",(mtmp && mtmp->mpeaceful && canspotmon(mtmp))?"true":"false");
        }
        printf(",\"name\":");m=glyph_to_mon(g);quoted(glyph_is_monster(g)&&m>=0?mons[m].mname:glyph_is_object(g)?object_name(g):"");
        if (glyph_is_object(g)) {
            object_type = glyph_is_body(g) ? CORPSE : glyph_to_obj(g);
            printf(",\"object\":{\"kind\":");quoted(object_kind(g));
            printf(",\"otyp\":%d,\"class\":%d,\"material\":%d,\"name\":",object_type,
                   object_type == CORPSE ? FOOD_CLASS : objects[object_type].oc_class,
                   object_type == CORPSE ? FLESH : objects[object_type].oc_material);
            quoted(object_name(g));putchar('}');
        }
        putchar('}');
    }
    puts("]}");fflush(stdout);
}
/* Input is one decimal keycode or a UTF-8 line, only after a request. */
static void read_request(const char *kind,const char *prompt,char *buf,int size) {
    frame();printf("{\"type\":\"request\",\"id\":%ld,\"kind\":",++request_id);quoted(kind);printf(",\"prompt\":");quoted(prompt);puts("}");fflush(stdout);
    if(!fgets(buf,size,stdin)) { hangup(0);exit(0); }
    buf[strcspn(buf,"\r\n")]=0;
}
static int key(const char *kind,const char *prompt) {char buf[BUFSZ];read_request(kind,prompt,buf,sizeof buf);int k=atoi(buf);return k>0&&k<256?k:27;}
static void noop(void) {}
static void strnoop(const char *s UNUSED) {}
static void intnoop(int i UNUSED) {}
static void init(int *a UNUSED,char **v UNUSED) {setvbuf(stdout,NULL,_IOLBF,0);for(int x=0;x<COLNO;x++)for(int y=0;y<ROWNO;y++)glyphs[x][y]=backgrounds[x][y]=-1;iflags.window_inited=TRUE;iflags.use_background_glyph=TRUE;}
static void name(void){Strcpy(plname,"Wanderer");}
static void finish(const char *s){event("ended",s);iflags.window_inited=FALSE;}
static winid create(int type){for(int i=1;i<BW;i++)if(!wins[i].type){wins[i].type=type;return i;}panic("bridge windows exhausted");return WIN_ERR;}
static void clear(winid w){if(w<1||w>=BW)return;for(int i=0;i<wins[w].n;i++)free(wins[w].items[i].text);wins[w].n=0;wins[w].prompt[0]=0;if(wins[w].type==NHW_MAP)for(int x=0;x<COLNO;x++)for(int y=0;y<ROWNO;y++)glyphs[x][y]=backgrounds[x][y]=-1;}
static void destroy(winid w){clear(w);if(w>0&&w<BW)wins[w].type=0;}
static void bridge_curs(winid w UNUSED,int x UNUSED,int y UNUSED){}
static void put(winid w,int attr UNUSED,const char *s){if(w>0&&w<BW&&(wins[w].type==NHW_TEXT||wins[w].type==NHW_MENU)){if(wins[w].n<BM){struct entry *e=&wins[w].items[wins[w].n++];e->text=strdup(s);e->selectable=FALSE;}}else event(w>0&&w<BW&&wins[w].type==NHW_STATUS?"status":"message",s);}
static void raw(const char *s){event("message",s);}
static void display(winid w,boolean block){if(w>0&&w<BW&&wins[w].n){printf("{\"type\":\"text\",\"lines\":[");for(int i=0;i<wins[w].n;i++){if(i)putchar(',');quoted(wins[w].items[i].text);}puts("]}");}if(block)key("more","Continue");}
static void file(
#ifdef FILE_AREAS
 const char *area,
#endif
 const char *path,boolean complain){
#ifdef FILE_AREAS
 dlb *f=dlb_fopen_area(area,path,"r");
#else
 dlb *f=dlb_fopen(path,"r");
#endif
 if(!f){if(complain)event("message","Could not open game text.");return;}winid w=create(NHW_TEXT);char s[BUFSZ];while(dlb_fgets(s,sizeof s,f))put(w,0,s);dlb_fclose(f);display(w,TRUE);destroy(w);}
static void add(winid w,int glyph UNUSED,int cnt UNUSED,const ANY_P *id,char accel,char group,int attr UNUSED,const char *s,unsigned int selected UNUSED){if(w<1||w>=BW||wins[w].n>=BM)return;struct entry *e=&wins[w].items[wins[w].n++];e->id=*id;e->selectable=id->a_void!=0;e->accelerator=accel;e->group_accelerator=group;e->text=strdup(s);}
static void end(winid w,const char *s){if(w>0&&w<BW)snprintf(wins[w].prompt,BUFSZ,"%s",s?s:"");}
static int bridge_select(winid w,int how,menu_item **out){char buf[BUFSZ];*out=NULL;if(w<1||w>=BW)return -1;
 printf("{\"type\":\"menu\",\"how\":%d,\"items\":[",how);for(int i=0;i<wins[w].n;i++){if(i)putchar(',');printf("{\"id\":%d,\"selectable\":%s,\"accelerator\":",i,wins[w].items[i].selectable?"true":"false");if(wins[w].items[i].accelerator) { char accel[2]={wins[w].items[i].accelerator,0};quoted(accel); } else quoted("");printf(",\"text\":");quoted(wins[w].items[i].text);putchar('}');}puts("]}");
 read_request("menu",wins[w].prompt,buf,sizeof buf);if(buf[0]=='!' )return -1;if(how==PICK_NONE)return 0;
 menu_item picked[BM];boolean seen[BM]={0};int n=0;char *p=strtok(buf,",");while(p){char *tail;long i=strtol(p,&tail,10);if(*tail==0&&i>=0&&i<wins[w].n&&wins[w].items[i].selectable&&!seen[i]){seen[i]=TRUE;picked[n].item=wins[w].items[i].id;picked[n++].count=-1;if(how==PICK_ONE)break;}p=strtok(NULL,",");}
 if(n){*out=(menu_item*)alloc(n*sizeof(menu_item));memcpy(*out,picked,n*sizeof(menu_item));}return n;}
static void glyph(winid w UNUSED,coordxy x,coordxy y,int g,int b){if(x>0&&x<COLNO&&y>=0&&y<ROWNO){glyphs[x][y]=g;if(cansee(x,y))backgrounds[x][y]=(b==NO_GLYPH?cmap_to_glyph(S_room):b);}}
static int getkey(void){return key("key","Command or direction");}
static int poskey(coordxy *x UNUSED,coordxy *y UNUSED,int *m UNUSED){return key("command","Your move");}
static char bridge_yn(const char *q,const char *choices,char def){char prompt[BUFSZ];snprintf(prompt,sizeof prompt,"%s [%s] (default: %c)",q,choices?choices:"any key",def?def:' ');for(;;){int k=key("key",prompt);if((k==13||k==10||k==' ')&&def)return def;if(k==27)return choices&&strchr(choices,'q')?'q':choices&&strchr(choices,'n')?'n':def?def:27;if(!choices||strchr(choices,k))return k;}}
static void line(const char *q,char *buf){read_request("line",q,buf,BUFSZ);}
static int ext(void){char buf[BUFSZ];line("Extended command",buf);for(int i=0;extcmdlist[i].ef_txt;i++)if(!strcmp(buf,extcmdlist[i].ef_txt)&&(!(extcmdlist[i].flags & WIZMODECMD)||wizard))return i;return -1;}
static void clip(int x UNUSED,int y UNUSED){}
static int prev(void){return 0;}
static void rip(winid w UNUSED,int how UNUSED){}
struct window_procs bridge_procs={
 .name="bridge",.wincap=WC_COLOR,.win_init_nhwindows=init,.win_player_selection=noop,.win_askname=name,.win_get_nh_event=noop,.win_exit_nhwindows=finish,.win_suspend_nhwindows=strnoop,.win_resume_nhwindows=noop,
 .win_create_nhwindow=create,.win_clear_nhwindow=clear,.win_display_nhwindow=display,.win_destroy_nhwindow=destroy,.win_curs=bridge_curs,.win_putstr=put,.win_display_file=file,.win_start_menu=clear,.win_add_menu=add,.win_end_menu=end,.win_select_menu=bridge_select,.win_message_menu=genl_message_menu,.win_update_inventory=noop,.win_mark_synch=noop,.win_wait_synch=noop,
#ifdef CLIPPING
 .win_cliparound=clip,
#endif
 .win_print_glyph=glyph,.win_raw_print=raw,.win_raw_print_bold=raw,.win_nhgetch=getkey,.win_nh_poskey=poskey,.win_nhbell=noop,.win_doprev_message=prev,.win_yn_function=bridge_yn,.win_getlin=line,.win_get_ext_cmd=ext,.win_number_pad=intnoop,.win_delay_output=noop,.win_start_screen=noop,.win_end_screen=noop,.win_outrip=rip,.win_preference_update=strnoop
};
