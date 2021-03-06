#include "CompileThreadBlock.hpp"

#include <cmath>

CompileThreadBlock::CompileThreadBlock(EditableMap *wmap, unsigned char **lightmap)
    : CompileThread(wmap, lightmap)
{
    thread_start();
}

CompileThreadBlock::~CompileThreadBlock() {
    thread_join();
}

void CompileThreadBlock::thread() {
    EditableMap::Lights& lights = wmap->get_light_sources();
    Tileset *ts = wmap->get_tileset_ptr();
    size_t nlgt = lights.size();
    int mw = wmap->get_width();
    int mh = wmap->get_height();
    int tw = ts->get_tile_width();
    int th = ts->get_tile_height();
    int w = mw * tw;
    int h = mh * th;
    short **pmap = wmap->get_map();
    short **pdeco = wmap->get_decoration();
    Point pr;

    for (size_t i = 0; i < nlgt; i++) {
        {
            ScopeMutex lock(mtx);
            finished_percent = 100 * (i + 1) / nlgt;
        }
        int r = lights[i]->radius;
        int lmaxsq = r * r;
        int lx = lights[i]->x;
        int ly = lights[i]->y;
        Point p2(static_cast<float>(lx * tw + (tw / 2)), static_cast<float>(ly * th + (th / 2)));
        int lsx = static_cast<int>(p2.x) - r;
        int lsy = static_cast<int>(p2.y) - r;
        int lex = static_cast<int>(p2.x) + r;
        int ley = static_cast<int>(p2.y) + r;
        if (lsx < 0) lsx = 0;
        if (lsy < 0) lsy = 0;
        if (lex > w) lex = w;
        if (ley > h) ley = h;

        int txs = lsx / tw;
        int txe = lex / tw;
        int tys = lsy / th;
        int tye = ley / th;

        for (int y = lsy; y < ley; y++) {
            for (int x = lsx; x < lex; x++) {
                int dindex = pdeco[y / th][x / tw];
                if (dindex < 0) {
                    lightmap[y][x * 4 + 3] = 0;
                } else {
                    bool contact = false;
                    Point p1(static_cast<float>(x), static_cast<float>(y));
                    float xd = p2.x - p1.x;
                    float yd = p2.y - p1.y;
                    float dist = xd * xd + yd * yd;
                    if (dist < lmaxsq) {
                        for (int tx = txs; tx < txe; tx++) {
                            for (int ty = tys; ty < tye; ty++) {
                                short index = pmap[ty][tx];
                                if (index >= 0) {
                                    if (ts->get_tile(index)->is_light_blocking()) {
                                        Point p1l(static_cast<float>(tx * tw), static_cast<float>(ty * th));
                                        Point p2l(static_cast<float>(tx * tw), static_cast<float>((ty + 1) * th - 0.5f));
                                        Point p1r(static_cast<float>((tx + 1) * tw - 0.5f), static_cast<float>(ty * th));
                                        Point p2r(static_cast<float>((tx + 1) * tw - 0.5f), static_cast<float>((ty + 1) * th - 0.5f));
                                        Point p1t(static_cast<float>(tx * tw), static_cast<float>(ty * th));
                                        Point p2t(static_cast<float>((tx + 1) * tw - 0.5f), static_cast<float>(ty * th));
                                        Point p1b(static_cast<float>(tx * tw), static_cast<float>((ty + 1) * th - 0.5f));
                                        Point p2b(static_cast<float>((tx + 1) * tw - 0.5f), static_cast<float>((ty + 1) * th - 0.5f));
                                        if (intersection(p1, p2, p1l, p2l, pr) ||
                                            intersection(p1, p2, p1r, p2r, pr) ||
                                            intersection(p1, p2, p1t, p2t, pr) ||
                                            intersection(p1, p2, p1b, p2b, pr))
                                        {
                                            contact = true;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (contact) {
                                break;
                            }
                        }
                    } else {
                        contact = true;
                    }
                    if (!contact) {
                        int v = static_cast<int>(sqrt(65025.0f * dist / lmaxsq));
                        if (v < lightmap[y][x * 4 + 3]) {
                            lightmap[y][x * 4 + 3] = v;
                        }
                    }
                }
            }
        }
    }

    ScopeMutex lock(mtx);
    finished = true;
}
