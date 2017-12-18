#include <cstdio>
#include <unordered_map>
#include <string>

#include "SDL.h"

#define KXVER 3
#include "k.h"

enum
{
    DEFAULT_WIDTH = 640,
    DEFAULT_HEIGHT = 480
};

namespace
{
    SDL_Window* window = nullptr;
    SDL_Renderer* windowRenderer = nullptr;
    bool is_init = false;

    const K kNull = nullptr;

    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        const Uint32 rmask = 0xff000000;
        const Uint32 gmask = 0x00ff0000;
        const Uint32 bmask = 0x0000ff00;
        const Uint32 amask = 0x000000ff;
    #else
        const Uint32 rmask = 0x000000ff;
        const Uint32 gmask = 0x0000ff00;
        const Uint32 bmask = 0x00ff0000;
        const Uint32 amask = 0xff000000;
    #endif

    const std::unordered_map<Uint32, const char*> event_type_str = {
        { SDL_QUIT, "quit" },
        { SDL_KEYDOWN, "keydown" },
        { SDL_KEYUP, "keyup" },
        { SDL_MOUSEMOTION, "mousemotion" },
        { SDL_MOUSEBUTTONDOWN, "mousebuttondown" },
        { SDL_MOUSEBUTTONUP, "mousebuttonup" },
        { SDL_MOUSEWHEEL, "mousewheel" },
    };

    const std::unordered_map<Uint32, const char*> kmod_type_str = {
        { KMOD_LSHIFT, "lshift" },
        { KMOD_RSHIFT, "rshift" },
        { KMOD_LCTRL, "lctrl" },
        { KMOD_RCTRL, "rctrl" },
        { KMOD_LALT, "lalt" },
        { KMOD_RALT, "ralt" },
        { KMOD_LGUI, "lgui" },
        { KMOD_RGUI, "rgui" },
        { KMOD_NUM, "num" },
        { KMOD_CAPS, "caps" },
        { KMOD_MODE, "mode" },
    };

    const std::unordered_map<Uint32, const char*> button_state_type_str = {
        { SDL_BUTTON_LMASK, "left" },
        { SDL_BUTTON_MMASK, "middle" },
        { SDL_BUTTON_RMASK, "right" },
        { SDL_BUTTON_X1MASK, "x1" },
        { SDL_BUTTON_X2MASK, "x2" },
    };

    const std::unordered_map<Uint32, const char*> button_type_str = {
        { SDL_BUTTON_LEFT, "left" },
        { SDL_BUTTON_MIDDLE, "middle" },
        { SDL_BUTTON_RIGHT, "right" },
        { SDL_BUTTON_X1, "x1" },
        { SDL_BUTTON_X2, "x2" },
    };

    std::unordered_map<I, SDL_Renderer*> render_handles;
}

template<typename MapType, typename KeyType, typename RetType>
auto get(const MapType& map, const KeyType k, RetType def)
{
    const auto found = map.find(k);
    if (found == map.cend())
    {
        return def;
    }

    return found->second;
}

template<typename MapType>
SDL_Renderer* get(const MapType& map, const K key)
{
    if (! key || key->t != -KI)
        return nullptr;
    return get(map, key->i, (SDL_Renderer*)nullptr);
}

extern "C" K sdl_quit(const K x)
{
    if (windowRenderer)
    {
        SDL_DestroyRenderer(windowRenderer);
        windowRenderer = nullptr;
        render_handles[1] = nullptr;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
    is_init = false;

    return kNull;
}

void clear_screen(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_SetRenderDrawColor(windowRenderer, r, g, b, a);
    SDL_RenderClear(windowRenderer);
}

void flush_screen()
{
    SDL_RenderPresent(windowRenderer);
}

extern "C" K sdl_init(const K x)
{
    sdl_quit(kNull);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "error: SDL_Init: %s\n", SDL_GetError());
        return krr("SDL_Init");
    }

    std::string title = "sdl2";
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;

    if (x && x->t == 0 && x->n == 3)
    {
        K t = kK(x)[0];
        K w = kK(x)[1];
        K h = kK(x)[2];

        if (t->t == KC)
        {
            title.assign((const char*)&kC(t)[0], t->n);
        }
        if (w->t == -KI)
        {
            width = w->i;
        }
        if (h->t == -KI)
        {
            height = h->i;
        }
    }

    #if (defined(__APPLE__)  || defined(__LINUX__))
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
    #endif

    if (SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_SHOWN, &window, &windowRenderer) != 0)
    {
        fprintf(stderr, "error: SDL_CreateWindowAndRenderer: %s\n", SDL_GetError());
        return krr("SDL_CreateWindowAndRenderer");
    }

    {
        const int c = SDL_GetNumRenderDrivers();
        for (int i = 0; i < c; ++i)
        {
            SDL_RendererInfo info;
            if (SDL_GetRenderDriverInfo(i, &info) == 0)
            {
                fprintf(stderr, "Render driver (%d): %s\n", i, info.name);
            }
        }

        SDL_RendererInfo cinfo;
        SDL_GetRendererInfo(windowRenderer, &cinfo);
        fprintf(stderr, "Current driver: %s\n", cinfo.name);
    }

    {
        Uint32 fmt = SDL_GetWindowPixelFormat(window);
        fprintf(stderr, "Window pixel format: %s\n", SDL_GetPixelFormatName(fmt));
    }

    SDL_SetWindowTitle(window, title.c_str());

    is_init = true;

    render_handles[1] = windowRenderer;

    clear_screen(0xff, 0xff, 0xff, 0xff);
    flush_screen();

    return kNull;
}

extern "C" K sdl_tick(const K x)
{
    if (is_init)
    {
        SDL_PumpEvents();
    }

    return kNull;
}

K translate_bitmask(const std::unordered_map<Uint32, const char*>&map, const Uint32 m)
{
    K n = ktn(KS, 0);
    for (int i = 0; i < (sizeof(m) * 8); i++)
    {
        const int mod = (1 << i);
        if ((int)m & mod)
        {
            const auto s = get(map, m, (const char*)nullptr);
            if (s)
            {
                js(&n, ss((char*)s));
            }
        }
    }
    return n;
}

K translate_button_state(const Uint16 m)
{
    return translate_bitmask(button_state_type_str, m);
}

K translate_keymod(const Uint16 m)
{
    return translate_bitmask(kmod_type_str, m);
}

K translate_event(const SDL_Event& e)
{
    SDL_CommonEvent* ecommon = (SDL_CommonEvent*)&e;

    S s_type = ss("etype");
    S s_typecode = ss("typecode");
    S s_timestamp = ss("timestamp");
    S s_data = ss("data");

    // Keys
    K ekeys = ktn(KS, 4);
    kS(ekeys)[0] = s_type;
    kS(ekeys)[1] = s_typecode;
    kS(ekeys)[2] = s_timestamp;
    kS(ekeys)[3] = s_data;

    // Values
    K evalues = ktn(0, 4);
    kK(evalues)[0] = ks(ss((char*)get(event_type_str, e.type, "")));
    kK(evalues)[1] = kj(static_cast<J>(e.type));
    kK(evalues)[2] = kj(static_cast<J>(ecommon->timestamp));

    K kdata = ktn(KS, 0);
    K data = ktn(0, 0);

    switch (e.type)
    {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            SDL_KeyboardEvent *ke = (SDL_KeyboardEvent*)&e;
            js(&kdata, ss("window_id"));    jk(&data, kj(static_cast<J>(ke->windowID)));
            js(&kdata, ss("state"));        jk(&data, ks(ke->state == SDL_PRESSED ? ss("pressed") : ss("released")));
            js(&kdata, ss("repeat"));       jk(&data, ki(static_cast<I>(ke->repeat)));
            js(&kdata, ss("scancode"));     jk(&data, ki(static_cast<I>(ke->keysym.scancode)));
            js(&kdata, ss("keycode"));      jk(&data, ki(static_cast<I>(ke->keysym.sym)));
            js(&kdata, ss("mod"));          jk(&data, ki(static_cast<I>(ke->keysym.mod)));
            js(&kdata, ss("keyname"));      jk(&data, ks(ss((char*)SDL_GetKeyName(ke->keysym.sym))));
            js(&kdata, ss("scancodename")); jk(&data, ks(ss((char*)SDL_GetScancodeName(ke->keysym.scancode))));
            js(&kdata, ss("modnames"));     jk(&data, translate_keymod(ke->keysym.mod)); 
        } break;

        case SDL_MOUSEMOTION:
        {
            SDL_MouseMotionEvent *mm = (SDL_MouseMotionEvent*)&e;
            js(&kdata, ss("window_id"));    jk(&data, kj(static_cast<J>(mm->windowID)));
            js(&kdata, ss("which"));        jk(&data, kj(static_cast<J>(mm->which)));
            js(&kdata, ss("state"));        jk(&data, translate_button_state(mm->state));
            js(&kdata, ss("x"));            jk(&data, ki(static_cast<I>(mm->x)));
            js(&kdata, ss("y"));            jk(&data, ki(static_cast<I>(mm->y)));
            js(&kdata, ss("xrel"));         jk(&data, ki(static_cast<I>(mm->xrel)));
            js(&kdata, ss("yrel"));         jk(&data, ki(static_cast<I>(mm->yrel)));
        } break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            SDL_MouseButtonEvent *mb = (SDL_MouseButtonEvent*)&e;
            js(&kdata, ss("window_id"));    jk(&data, kj(static_cast<J>(mb->windowID)));
            js(&kdata, ss("which"));        jk(&data, kj(static_cast<J>(mb->which)));
            js(&kdata, ss("button"));       jk(&data, ks(ss((char*)get(button_type_str, mb->button, ""))));
            js(&kdata, ss("state"));        jk(&data, ks(mb->state == SDL_PRESSED ? ss("pressed") : ss("released")));
            js(&kdata, ss("clicks"));       jk(&data, ki(static_cast<I>(mb->clicks)));
            js(&kdata, ss("x"));            jk(&data, ki(static_cast<I>(mb->x)));
            js(&kdata, ss("y"));            jk(&data, ki(static_cast<I>(mb->y)));
        } break;

        case SDL_MOUSEWHEEL:
        {
            SDL_MouseWheelEvent *mw = (SDL_MouseWheelEvent*)&e;
            js(&kdata, ss("window_id"));    jk(&data, kj(static_cast<J>(mw->windowID)));
            js(&kdata, ss("which"));        jk(&data, kj(static_cast<J>(mw->which)));
            js(&kdata, ss("x"));            jk(&data, ki(static_cast<I>(mw->x)));
            js(&kdata, ss("y"));            jk(&data, ki(static_cast<I>(mw->y)));
            js(&kdata, ss("direction"));    jk(&data, ks(mw->direction == SDL_MOUSEWHEEL_NORMAL ? ss("normal") : ss("flipped")));
        } break;
    }

    kK(evalues)[3] = xD(kdata, data);

    return xD(ekeys, evalues);
}

extern "C" K sdl_poll_event(const K x)
{
    SDL_Event event;
    K evt = ktn(0, 0);

    while(SDL_PollEvent(&event) != 0)
    {
        K e = translate_event(event);
        jk(&evt, e);
    }

    return evt;
}

extern "C" K sdl_show_simple_message_box(const K typ, const K title, const K message, const K win_id)
{
    Uint32 flags = 0;
    if (typ && typ->t == KS)
    {
        static std::unordered_map<std::string, Uint32> typ_to_flags = {
            { "info", SDL_MESSAGEBOX_INFORMATION },
            { "warn", SDL_MESSAGEBOX_WARNING },
            { "error", SDL_MESSAGEBOX_ERROR },
        };
        auto it = typ_to_flags.find(typ->s);
        if (it != typ_to_flags.cend())
        {
            flags = it->second;
        }
    }
    std::string boxtitle = "";
    std::string boxmessage = "";

    if (title && title->t == KC)
    {
        boxtitle.assign((char*)&kC(title)[0], title->n);
    }
    if (message && message->t == KC)
    {
        boxmessage.assign((char*)&kC(message)[0], message->n);
    }

    SDL_Window* w = nullptr;
    if (win_id && win_id->t == KJ)
    {
        w = SDL_GetWindowFromID((Uint32)win_id->j);
    }
    
    SDL_ShowSimpleMessageBox(flags, boxtitle.c_str(), boxmessage.c_str(), w);

    return kNull;
}

struct RGBA8
{
    Uint8 r, g, b, a;

    RGBA8(const K rgba)
    {
        if (rgba && rgba->t == KG && rgba->n >= 3)
        {
            r = kG(rgba)[0];
            g = kG(rgba)[1];
            b = kG(rgba)[2];
            if (rgba->n >= 4)
            {
                a = kG(rgba)[3];
            }
        }
    }
};

SDL_Renderer* get_or_default(const K id)
{
    return (id ? get(render_handles, id) : windowRenderer);
}

extern "C" K sdl_render_clear(const K id, const K rgba)
{
    auto r = get_or_default(id);
    if (! r) return krr("handle");

    RGBA8 c(rgba);
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderClear(r);

    return kNull;
}

extern "C" K sdl_render_present(const K id)
{
    auto r = get_or_default(id);
    if (! r) return krr("handle");

    SDL_RenderPresent(r);

    return kNull;
}

template<decltype(SDL_RenderDrawPoints) DrawFN>
struct draw_wrapper_points {
    int operator()(SDL_Renderer* r, const SDL_Point* p, int c)
    {
        return DrawFN(r, p, c);
    }
};

static_assert(sizeof(SDL_Rect::w) == sizeof(I) && sizeof(SDL_Rect::h) == sizeof(I), "incompatible struct size type aliasing");
template<decltype(SDL_RenderDrawRects) DrawFN>
struct draw_wrapper_rect {
    int operator()(SDL_Renderer* r, const SDL_Rect* p, int c)
    {
        return DrawFN(r, p, c);
    }
};

template<typename FN, typename PT, int M>
K render_draw_FN(const K id, const K rgba, const K points)
{
    auto r = get_or_default(id);
    if (! r) return krr("handle");
    if (! rgba) return krr("rgba");

    static_assert(sizeof(decltype(PT::x)) == sizeof(I), "incompatible size type aliasing");
    static_assert(sizeof(PT) == M * sizeof(I), "incompatible struct size type aliasing");

    if (! points || points->t != KI || points->n % M != 0) return krr("points");

    const PT* p = (PT*)&kI(points)[0];
    const int count = points->n / M;
    const RGBA8 c(rgba);

    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_SetRenderDrawBlendMode(r, c.a == 0xff ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
    Uint32 tstart = SDL_GetTicks();
    FN f; f(r, p, count);
    Uint32 tend = SDL_GetTicks();
    fprintf(stderr, "render_draw_FN took: %u ms count: %d\n", tend - tstart, count);

    return kNull;
}

extern "C" K sdl_render_draw_points(const K id, const K rgba, const K points)
{
    return render_draw_FN<draw_wrapper_points<SDL_RenderDrawPoints>, SDL_Point, 2>(id, rgba, points);
}

extern "C" K sdl_render_draw_lines(const K id, const K rgba, const K points)
{
    return render_draw_FN<draw_wrapper_points<SDL_RenderDrawLines>, SDL_Point, 2>(id, rgba, points);
}

extern "C" K sdl_render_draw_rects(const K id, const K rgba, const K rects)
{
    return render_draw_FN<draw_wrapper_rect<SDL_RenderDrawRects>, SDL_Rect, 4>(id, rgba, rects);
}

extern "C" K sdl_render_fill_rects(const K id, const K rgba, const K rects)
{
    return render_draw_FN<draw_wrapper_rect<SDL_RenderFillRects>, SDL_Rect, 4>(id, rgba, rects);
}
