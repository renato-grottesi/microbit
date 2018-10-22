/*
 * Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 * Copyright (c) 2016 Renato Grottesi renato.grottesi@gmail.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * The SVG parser is based on Anti-Grain Geometry 2.4 SVG example
 * Copyright (C) 2002-2004 Maxim Shemanarev (McSeem) (http://www.antigrain.com/)
 *
 * Arc calculation code based on canvg (https://code.google.com/p/canvg/)
 *
 * Limitations: the transform attribute must come before the other attributes.
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

// Lenght proportional to radius of a cubic bezier handle for 90deg arcs.
#define NSVG_KAPPA90 (0.5522847493f)
#define NSVG_PI (3.14159265358979323846264338327f)
#define NSVG_EPSILON (1e-12)
#define NSVG_MAX_XFORMS 64

typedef void (*nsvgMoveToCb)(float, float);
typedef void (*nsvgBeginPathCb)();
typedef void (*nsvgEndPathCb)();

typedef struct NSVGxform
{
    float xform[6];
} NSVGxform;

typedef struct NSVGCommonAttrs
{
    float cx;
    float cy;
    float height;
    float r;
    float rx;
    float ry;
    float width;
    float x;
    float x1;
    float x2;
    float y;
    float y1;
    float y2;
} NSVGCommonAttrs;

typedef struct NSVGparser
{
    NSVGxform xforms[NSVG_MAX_XFORMS];
    int xformCurrent;
    float *pts;
    int npts;
    float sx;
    float sy;
    int cpts;
    char pathFlag;
    nsvgMoveToCb moveToCb;
    float mtx; // move to last entries
    float mty;
    nsvgBeginPathCb beginPathCb;
    nsvgEndPathCb endPathCb;
    float width;
    float height;
    float scale;
    float x0;
    float y0;
    NSVGCommonAttrs attrs;
} NSVGparser;

static int nsvg__isspace(char c)
{
    return strchr(" \t\n\v\f\r", c) != 0;
}

static int nsvg__isdigit(char c)
{
    return strchr("0123456789", c) != 0;
}

static int nsvg__isnum(char c)
{
    return strchr("0123456789+-.eE", c) != 0;
}

static void nsvg__xformIdentity(float *t)
{
    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = 1.0f;
    t[4] = 0.0f;
    t[5] = 0.0f;
}

static void nsvg__xformSetTranslation(float *t, float tx, float ty)
{
    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = 1.0f;
    t[4] = tx;
    t[5] = ty;
}

static void nsvg__xformSetScale(float *t, float sx, float sy)
{
    t[0] = sx;
    t[1] = 0.0f;
    t[2] = 0.0f;
    t[3] = sy;
    t[4] = 0.0f;
    t[5] = 0.0f;
}

static void nsvg__xformSetSkewX(float *t, float a)
{
    t[0] = 1.0f;
    t[1] = 0.0f;
    t[2] = tanf(a);
    t[3] = 1.0f;
    t[4] = 0.0f;
    t[5] = 0.0f;
}

static void nsvg__xformSetSkewY(float *t, float a)
{
    t[0] = 1.0f;
    t[1] = tanf(a);
    t[2] = 0.0f;
    t[3] = 1.0f;
    t[4] = 0.0f;
    t[5] = 0.0f;
}

static void nsvg__xformSetRotation(float *t, float a)
{
    float cs = cosf(a), sn = sinf(a);
    t[0] = cs;
    t[1] = sn;
    t[2] = -sn;
    t[3] = cs;
    t[4] = 0.0f;
    t[5] = 0.0f;
}

static void nsvg__xformMultiply(float *t, float *s)
{
    float t0 = t[0] * s[0] + t[1] * s[2];
    float t2 = t[2] * s[0] + t[3] * s[2];
    float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
    t[1] = t[0] * s[1] + t[1] * s[3];
    t[3] = t[2] * s[1] + t[3] * s[3];
    t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
    t[0] = t0;
    t[2] = t2;
    t[4] = t4;
}

static void nsvg__xformPremultiply(float *t, float *s)
{
    float s2[6];
    memcpy(s2, s, sizeof(float) * 6);
    nsvg__xformMultiply(s2, t);
    memcpy(t, s2, sizeof(float) * 6);
}

static void nsvg__xformPoint(float *dx, float *dy, float x, float y, float *t)
{
    *dx = x * t[0] + y * t[2] + t[4];
    *dy = x * t[1] + y * t[3] + t[5];
}

static void nsvg__xformVec(float *dx, float *dy, float x, float y, float *t)
{
    *dx = x * t[0] + y * t[2];
    *dy = x * t[1] + y * t[3];
}

static void nsvg__resetPath(NSVGparser *p)
{
    p->npts = 0;
}

static void nsvg__addPoint(NSVGparser *p, float x, float y)
{
    if (p->npts + 1 > p->cpts)
    {
        p->cpts = p->cpts ? p->cpts * 2 : 8;
        p->pts = (float *)realloc(p->pts, p->cpts * 2 * sizeof(float));
        if (!p->pts)
            return;
    }
    p->pts[p->npts * 2 + 0] = x;
    p->pts[p->npts * 2 + 1] = y;
    p->npts++;
}

static void nsvg__moveTo(NSVGparser *p, float x, float y)
{
    if (p->npts > 0)
    {
        p->pts[(p->npts - 1) * 2 + 0] = x;
        p->pts[(p->npts - 1) * 2 + 1] = y;
    }
    else
    {
        nsvg__addPoint(p, x, y);
        p->sx = x;
        p->sy = y;
    }
}

static void nsvg__lineTo(NSVGparser *p, float x, float y)
{
    float px, py, dx, dy;
    if (p->npts > 0)
    {
        px = p->pts[(p->npts - 1) * 2 + 0];
        py = p->pts[(p->npts - 1) * 2 + 1];
        dx = x - px;
        dy = y - py;
        nsvg__addPoint(p, px + dx / 3.0f, py + dy / 3.0f);
        nsvg__addPoint(p, x - dx / 3.0f, y - dy / 3.0f);
        nsvg__addPoint(p, x, y);
    }
}

static void nsvg__cubicBezTo(NSVGparser *p, float cpx1, float cpy1, float cpx2, float cpy2, float x, float y)
{
    nsvg__addPoint(p, cpx1, cpy1);
    nsvg__addPoint(p, cpx2, cpy2);
    nsvg__addPoint(p, x, y);
}

static NSVGxform *nsvg__getCurrentXForm(NSVGparser *p)
{
    return &p->xforms[p->xformCurrent];
}

static void nsvg__pushAttr(NSVGparser *p)
{
    if (p->xformCurrent < NSVG_MAX_XFORMS - 1)
    {
        p->xformCurrent++;
        memcpy(&p->xforms[p->xformCurrent], &p->xforms[p->xformCurrent - 1], sizeof(NSVGxform));
    }
}

static void nsvg__popAttr(NSVGparser *p)
{
    if (p->xformCurrent > 0)
        p->xformCurrent--;
}

static float nsvg__distPtSeg(float x, float y, float px, float py, float qx, float qy)
{
    float pqx, pqy, dx, dy, d, t;
    pqx = qx - px;
    pqy = qy - py;
    dx = x - px;
    dy = y - py;
    d = pqx * pqx + pqy * pqy;
    t = pqx * dx + pqy * dy;
    if (d > 0)
        t /= d;
    if (t < 0)
        t = 0;
    else if (t > 1)
        t = 1;
    dx = px + t * pqx - x;
    dy = py + t * pqy - y;
    return dx * dx + dy * dy;
}

static void nsvg__cubicBez(NSVGparser *p, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tol,
                           int level)
{
    float x12, y12, x23, y23, x34, y34, x123, y123, x234, y234, x1234, y1234;
    float d;

    if (level > 12)
        return;

    x12 = (x1 + x2) * 0.5f;
    y12 = (y1 + y2) * 0.5f;
    x23 = (x2 + x3) * 0.5f;
    y23 = (y2 + y3) * 0.5f;
    x34 = (x3 + x4) * 0.5f;
    y34 = (y3 + y4) * 0.5f;
    x123 = (x12 + x23) * 0.5f;
    y123 = (y12 + y23) * 0.5f;
    x234 = (x23 + x34) * 0.5f;
    y234 = (y23 + y34) * 0.5f;
    x1234 = (x123 + x234) * 0.5f;
    y1234 = (y123 + y234) * 0.5f;

    d = nsvg__distPtSeg(x1234, y1234, x1, y1, x4, y4);
    if (d > tol * tol)
    {
        nsvg__cubicBez(p, x1, y1, x12, y12, x123, y123, x1234, y1234, tol, level + 1);
        nsvg__cubicBez(p, x1234, y1234, x234, y234, x34, y34, x4, y4, tol, level + 1);
    }
    else
    {
        if (x4 > 0.0 && x4 < 1.0 && y4 > 0.0 && y4 < 4.0)
            p->moveToCb(x4, y4);
        p->mtx = x4;
        p->mty = y4;
    }
}

static void nsvg__drawPath(NSVGparser *p, char closed)
{
    float pts[8]; // Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...

    NSVGxform *xform = nsvg__getCurrentXForm(p);
    int i;

    if (p->npts < 4)
        return;

    if (closed)
        nsvg__lineTo(p, p->sx, p->sy);

    float orig_x = 0.0;
    float orig_y = 0.0;

    int idx = 0;
    float norm_f = p->width > p->height ? p->width : p->height;
    // Transform path.
    for (i = 0; i < p->npts; ++i)
    {
        nsvg__xformPoint(&pts[(i * 2) % 8], &pts[(i * 2 + 1) % 8], p->pts[i * 2], p->pts[i * 2 + 1], xform->xform);
        pts[(i * 2 + 0) % 8] = ((pts[(i * 2 + 0) % 8] / norm_f) * p->scale) + p->x0;
        pts[(i * 2 + 1) % 8] = ((pts[(i * 2 + 1) % 8] / norm_f) * p->scale) + p->y0;
        /* First point starts the drawing */
        if (i == 0)
        {
            orig_x = pts[0];
            orig_y = pts[1];
            if (p->mtx != orig_x || p->mty != orig_y)
            {
                p->endPathCb();

                if (orig_x > 0.0 && orig_x < 1.0 && orig_y > 0.0 && orig_y < 4.0)
                    p->moveToCb(orig_x, orig_y);
                p->mtx = orig_x;
                p->mty = orig_y;
            }
            p->beginPathCb();
        }

        /* If we have at least 4 points */
        if (i >= 3)
        {
            /* A bezier every 3 points, linking with one from before */
            if ((i % 3) == 0)
            {
                nsvg__cubicBez(p, pts[(0 + idx) % 8], pts[(1 + idx) % 8], pts[(2 + idx) % 8], pts[(3 + idx) % 8], pts[(4 + idx) % 8],
                               pts[(5 + idx) % 8], pts[(6 + idx) % 8], pts[(7 + idx) % 8], 0.00015 * p->scale, 0);
                idx = (idx + 8 - 2) % 8;
            }
        }
    }

    if (closed)
    {
        p->mtx = orig_x;
        p->mty = orig_y;
    }

    return;
}

static int nsvg__parseNumber(FILE *fp, char quote, int begin, char *it, const int size)
{
    const int last = size - 1;
    int i = 0;
    int s = begin;

    // sign
    if (s == '-' || s == '+')
    {
        if (i < last)
            it[i++] = s;
        s = fgetc(fp);
    }
    // integer part
    while (s != EOF && s != quote && nsvg__isdigit(s))
    {
        if (i < last)
            it[i++] = s;
        s = fgetc(fp);
    }
    if (s == '.')
    {
        // decimal point
        if (i < last)
            it[i++] = s;
        s = fgetc(fp);
        // fraction part
        while (s != EOF && s != quote && nsvg__isdigit(s))
        {
            if (i < last)
                it[i++] = s;
            s = fgetc(fp);
        }
    }
    // exponent
    if (s == 'e' || s == 'E')
    {
        if (i < last)
            it[i++] = s;
        s = fgetc(fp);
        if (s == '-' || s == '+')
        {
            if (i < last)
                it[i++] = s;
            s = fgetc(fp);
        }
        while (s != EOF && s != quote && nsvg__isdigit(s))
        {
            if (i < last)
                it[i++] = s;
            s = fgetc(fp);
        }
    }
    it[i] = '\0';
    return s;
}

static float nsvg__parseCoordinate(FILE *fp, char quote)
{
    static char attr_value[32];
    int s = fgetc(fp);
    // Store value and find the end of it.
    int attr_value_len = 0;
    while (s != EOF && s != '>' && s != quote)
    {
        // Only save up to the buffer, but skip to the next quote
        if (attr_value_len < (32 - 2))
        {
            attr_value[attr_value_len++] = s;
        }
        s = fgetc(fp);
    }
    attr_value[attr_value_len++] = '\0';

    float value = 0.0f;
    char units[32] = "";
    sscanf(attr_value, "%f%s", &value, units);
    return value;
}

static int nsvg__parseTransformArgs(FILE *fp, char quote, float *args, int maxNa, int *na)
{
    char it[64];
    int s = fgetc(fp);

    *na = 0;
    while (s != EOF && s != quote && s != '(')
        s = fgetc(fp);
    if (s == EOF || s == quote)
        return s;

    while (s != EOF && s != quote && s != ')')
    {
        if (s == '-' || s == '+' || s == '.' || nsvg__isdigit(s))
        {
            if (*na >= maxNa)
                return s;
            s = nsvg__parseNumber(fp, quote, s, it, 64);
            args[(*na)++] = (float)atof(it);
        }
        else
        {
            s = fgetc(fp);
        }
    }
    return s;
}

static int nsvg__parseMatrix(float *xform, FILE *fp, char quote)
{
    float t[6];
    int na = 0;
    int s = nsvg__parseTransformArgs(fp, quote, t, 6, &na);
    if (na != 6)
        return s;
    memcpy(xform, t, sizeof(float) * 6);
    return s;
}

static int nsvg__parseTranslate(float *xform, FILE *fp, char quote)
{
    float args[2];
    float t[6];
    int na = 0;
    int s = nsvg__parseTransformArgs(fp, quote, args, 2, &na);
    if (na == 1)
        args[1] = 0.0;

    nsvg__xformSetTranslation(t, args[0], args[1]);
    memcpy(xform, t, sizeof(float) * 6);
    return s;
}

static int nsvg__parseScale(float *xform, FILE *fp, char quote)
{
    float args[2];
    int na = 0;
    float t[6];
    int s = nsvg__parseTransformArgs(fp, quote, args, 2, &na);
    if (na == 1)
        args[1] = args[0];
    nsvg__xformSetScale(t, args[0], args[1]);
    memcpy(xform, t, sizeof(float) * 6);
    return s;
}

static int nsvg__parseSkewX(float *xform, FILE *fp, char quote)
{
    float args[1];
    int na = 0;
    float t[6];
    int s = nsvg__parseTransformArgs(fp, quote, args, 1, &na);
    nsvg__xformSetSkewX(t, args[0] / 180.0f * NSVG_PI);
    memcpy(xform, t, sizeof(float) * 6);
    return s;
}

static int nsvg__parseSkewY(float *xform, FILE *fp, char quote)
{
    float args[1];
    int na = 0;
    float t[6];
    int s = nsvg__parseTransformArgs(fp, quote, args, 1, &na);
    nsvg__xformSetSkewY(t, args[0] / 180.0f * NSVG_PI);
    memcpy(xform, t, sizeof(float) * 6);
    return s;
}

static int nsvg__parseRotate(float *xform, FILE *fp, char quote)
{
    float args[3];
    int na = 0;
    float m[6];
    float t[6];
    int s = nsvg__parseTransformArgs(fp, quote, args, 3, &na);
    if (na == 1)
        args[1] = args[2] = 0.0f;
    nsvg__xformIdentity(m);

    if (na > 1)
    {
        nsvg__xformSetTranslation(t, -args[1], -args[2]);
        nsvg__xformMultiply(m, t);
    }

    nsvg__xformSetRotation(t, args[0] / 180.0f * NSVG_PI);
    nsvg__xformMultiply(m, t);

    if (na > 1)
    {
        nsvg__xformSetTranslation(t, args[1], args[2]);
        nsvg__xformMultiply(m, t);
    }

    memcpy(xform, m, sizeof(float) * 6);
    return s;
}

static int nsvg__parseTransform(float *xform, FILE *fp, char quote)
{
    int s;
    float t[6];

    nsvg__xformIdentity(xform);
    while (1)
    {
        s = fgetc(fp);

        if (s == EOF || s == '>' || s == quote)
            break;

        if (s == 'm')
        {
            if (fgetc(fp) != 'a')
                continue;
            if (fgetc(fp) != 't')
                continue;
            if (fgetc(fp) != 'r')
                continue;
            if (fgetc(fp) != 'i')
                continue;
            if (fgetc(fp) != 'x')
                continue;
            s = nsvg__parseMatrix(t, fp, quote);
            nsvg__xformPremultiply(xform, t);
        }
        else if (s == 'r')
        {
            if (fgetc(fp) != 'o')
                continue;
            if (fgetc(fp) != 't')
                continue;
            if (fgetc(fp) != 'a')
                continue;
            if (fgetc(fp) != 't')
                continue;
            if (fgetc(fp) != 'e')
                continue;
            s = nsvg__parseRotate(t, fp, quote);
            nsvg__xformPremultiply(xform, t);
        }
        else if (s == 't')
        {
            if (fgetc(fp) != 'r')
                continue;
            if (fgetc(fp) != 'a')
                continue;
            if (fgetc(fp) != 'n')
                continue;
            if (fgetc(fp) != 's')
                continue;
            if (fgetc(fp) != 'l')
                continue;
            if (fgetc(fp) != 'a')
                continue;
            if (fgetc(fp) != 't')
                continue;
            if (fgetc(fp) != 'e')
                continue;
            s = nsvg__parseTranslate(t, fp, quote);
            nsvg__xformPremultiply(xform, t);
        }
        else if (s == 's')
        {
            if (fgetc(fp) == 'c')
            {
                if (fgetc(fp) != 'a')
                    continue;
                if (fgetc(fp) != 'l')
                    continue;
                if (fgetc(fp) != 'e')
                    continue;
                s = nsvg__parseScale(t, fp, quote);
                nsvg__xformPremultiply(xform, t);
            }
            if (fgetc(fp) == 'k')
            {
                if (fgetc(fp) != 'e')
                    continue;
                if (fgetc(fp) != 'w')
                    continue;

                if (fgetc(fp) == 'X')
                {
                    s = nsvg__parseSkewX(t, fp, quote);
                    nsvg__xformPremultiply(xform, t);
                }
                else if (fgetc(fp) == 'Y')
                {
                    s = nsvg__parseSkewY(t, fp, quote);
                    nsvg__xformPremultiply(xform, t);
                }
                else
                    continue;
            }
            else
                continue;
        }
    }

    return s;
}

static int nsvg__getArgsPerElement(char cmd)
{
    switch (cmd)
    {
    case 'v':
    case 'V':
    case 'h':
    case 'H':
        return 1;
    case 'm':
    case 'M':
    case 'l':
    case 'L':
    case 't':
    case 'T':
        return 2;
    case 'q':
    case 'Q':
    case 's':
    case 'S':
        return 4;
    case 'c':
    case 'C':
        return 6;
    case 'a':
    case 'A':
        return 7;
    }
    return 0;
}

static void nsvg__pathMoveTo(NSVGparser *p, float *cpx, float *cpy, float *args, int rel)
{
    if (rel)
    {
        *cpx += args[0];
        *cpy += args[1];
    }
    else
    {
        *cpx = args[0];
        *cpy = args[1];
    }
    nsvg__moveTo(p, *cpx, *cpy);
}

static void nsvg__pathLineTo(NSVGparser *p, float *cpx, float *cpy, float *args, int rel)
{
    if (rel)
    {
        *cpx += args[0];
        *cpy += args[1];
    }
    else
    {
        *cpx = args[0];
        *cpy = args[1];
    }
    nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathHLineTo(NSVGparser *p, float *cpx, float *cpy, float *args, int rel)
{
    if (rel)
        *cpx += args[0];
    else
        *cpx = args[0];
    nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathVLineTo(NSVGparser *p, float *cpx, float *cpy, float *args, int rel)
{
    if (rel)
        *cpy += args[0];
    else
        *cpy = args[0];
    nsvg__lineTo(p, *cpx, *cpy);
}

static void nsvg__pathCubicBezTo(NSVGparser *p, float *cpx, float *cpy, float *cpx2, float *cpy2, float *args, int rel)
{
    float x2, y2, cx1, cy1, cx2, cy2;

    if (rel)
    {
        cx1 = *cpx + args[0];
        cy1 = *cpy + args[1];
        cx2 = *cpx + args[2];
        cy2 = *cpy + args[3];
        x2 = *cpx + args[4];
        y2 = *cpy + args[5];
    }
    else
    {
        cx1 = args[0];
        cy1 = args[1];
        cx2 = args[2];
        cy2 = args[3];
        x2 = args[4];
        y2 = args[5];
    }

    nsvg__cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

    *cpx2 = cx2;
    *cpy2 = cy2;
    *cpx = x2;
    *cpy = y2;
}

static void nsvg__pathCubicBezShortTo(NSVGparser *p, float *cpx, float *cpy, float *cpx2, float *cpy2, float *args, int rel)
{
    float x1, y1, x2, y2, cx1, cy1, cx2, cy2;

    x1 = *cpx;
    y1 = *cpy;
    if (rel)
    {
        cx2 = *cpx + args[0];
        cy2 = *cpy + args[1];
        x2 = *cpx + args[2];
        y2 = *cpy + args[3];
    }
    else
    {
        cx2 = args[0];
        cy2 = args[1];
        x2 = args[2];
        y2 = args[3];
    }

    cx1 = 2 * x1 - *cpx2;
    cy1 = 2 * y1 - *cpy2;

    nsvg__cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

    *cpx2 = cx2;
    *cpy2 = cy2;
    *cpx = x2;
    *cpy = y2;
}

static void nsvg__pathQuadBezTo(NSVGparser *p, float *cpx, float *cpy, float *cpx2, float *cpy2, float *args, int rel)
{
    float x1, y1, x2, y2, cx, cy;
    float cx1, cy1, cx2, cy2;

    x1 = *cpx;
    y1 = *cpy;
    if (rel)
    {
        cx = *cpx + args[0];
        cy = *cpy + args[1];
        x2 = *cpx + args[2];
        y2 = *cpy + args[3];
    }
    else
    {
        cx = args[0];
        cy = args[1];
        x2 = args[2];
        y2 = args[3];
    }

    // Convert to cubic bezier
    cx1 = x1 + 2.0f / 3.0f * (cx - x1);
    cy1 = y1 + 2.0f / 3.0f * (cy - y1);
    cx2 = x2 + 2.0f / 3.0f * (cx - x2);
    cy2 = y2 + 2.0f / 3.0f * (cy - y2);

    nsvg__cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

    *cpx2 = cx;
    *cpy2 = cy;
    *cpx = x2;
    *cpy = y2;
}

static void nsvg__pathQuadBezShortTo(NSVGparser *p, float *cpx, float *cpy, float *cpx2, float *cpy2, float *args, int rel)
{
    float x1, y1, x2, y2, cx, cy;
    float cx1, cy1, cx2, cy2;

    x1 = *cpx;
    y1 = *cpy;
    if (rel)
    {
        x2 = *cpx + args[0];
        y2 = *cpy + args[1];
    }
    else
    {
        x2 = args[0];
        y2 = args[1];
    }

    cx = 2 * x1 - *cpx2;
    cy = 2 * y1 - *cpy2;

    // Convert to cubix bezier
    cx1 = x1 + 2.0f / 3.0f * (cx - x1);
    cy1 = y1 + 2.0f / 3.0f * (cy - y1);
    cx2 = x2 + 2.0f / 3.0f * (cx - x2);
    cy2 = y2 + 2.0f / 3.0f * (cy - y2);

    nsvg__cubicBezTo(p, cx1, cy1, cx2, cy2, x2, y2);

    *cpx2 = cx;
    *cpy2 = cy;
    *cpx = x2;
    *cpy = y2;
}

static float nsvg__sqr(float x)
{
    return x * x;
}
static float nsvg__vmag(float x, float y)
{
    return sqrtf(x * x + y * y);
}

static float nsvg__vecrat(float ux, float uy, float vx, float vy)
{
    return (ux * vx + uy * vy) / (nsvg__vmag(ux, uy) * nsvg__vmag(vx, vy));
}

static float nsvg__vecang(float ux, float uy, float vx, float vy)
{
    float r = nsvg__vecrat(ux, uy, vx, vy);
    if (r < -1.0f)
        r = -1.0f;
    if (r > 1.0f)
        r = 1.0f;
    return ((ux * vy < uy * vx) ? -1.0f : 1.0f) * acosf(r);
}

static void nsvg__pathArcTo(NSVGparser *p, float *cpx, float *cpy, float *args, int rel)
{
    // Ported from canvg (https://code.google.com/p/canvg/)
    float rx, ry, rotx;
    float x1, y1, x2, y2, cx, cy, dx, dy, d;
    float x1p, y1p, cxp, cyp, s, sa, sb;
    float ux, uy, vx, vy, a1, da;
    float x, y, tanx, tany, a, px = 0, py = 0, ptanx = 0, ptany = 0, t[6];
    float sinrx, cosrx;
    int fa, fs;
    int i, ndivs;
    float hda, kappa;

    rx = fabsf(args[0]);                // y radius
    ry = fabsf(args[1]);                // x radius
    rotx = args[2] / 180.0f * NSVG_PI;  // x rotation engle
    fa = fabsf(args[3]) > 1e-6 ? 1 : 0; // Large arc
    fs = fabsf(args[4]) > 1e-6 ? 1 : 0; // Sweep direction
    x1 = *cpx;                          // start point
    y1 = *cpy;
    if (rel)
    { // end point
        x2 = *cpx + args[5];
        y2 = *cpy + args[6];
    }
    else
    {
        x2 = args[5];
        y2 = args[6];
    }

    dx = x1 - x2;
    dy = y1 - y2;
    d = sqrtf(dx * dx + dy * dy);
    if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f)
    {
        // The arc degenerates to a line
        nsvg__lineTo(p, x2, y2);
        *cpx = x2;
        *cpy = y2;
        return;
    }

    sinrx = sinf(rotx);
    cosrx = cosf(rotx);

    // Convert to center point parameterization.
    // http://www.w3.org/TR/SVG11/implnote.html#ArcImplementationNotes
    // 1) Compute x1', y1'
    x1p = cosrx * dx / 2.0f + sinrx * dy / 2.0f;
    y1p = -sinrx * dx / 2.0f + cosrx * dy / 2.0f;
    d = nsvg__sqr(x1p) / nsvg__sqr(rx) + nsvg__sqr(y1p) / nsvg__sqr(ry);
    if (d > 1)
    {
        d = sqrtf(d);
        rx *= d;
        ry *= d;
    }
    // 2) Compute cx', cy'
    s = 0.0f;
    sa = nsvg__sqr(rx) * nsvg__sqr(ry) - nsvg__sqr(rx) * nsvg__sqr(y1p) - nsvg__sqr(ry) * nsvg__sqr(x1p);
    sb = nsvg__sqr(rx) * nsvg__sqr(y1p) + nsvg__sqr(ry) * nsvg__sqr(x1p);
    if (sa < 0.0f)
        sa = 0.0f;
    if (sb > 0.0f)
        s = sqrtf(sa / sb);
    if (fa == fs)
        s = -s;
    cxp = s * rx * y1p / ry;
    cyp = s * -ry * x1p / rx;

    // 3) Compute cx,cy from cx',cy'
    cx = (x1 + x2) / 2.0f + cosrx * cxp - sinrx * cyp;
    cy = (y1 + y2) / 2.0f + sinrx * cxp + cosrx * cyp;

    // 4) Calculate theta1, and delta theta.
    ux = (x1p - cxp) / rx;
    uy = (y1p - cyp) / ry;
    vx = (-x1p - cxp) / rx;
    vy = (-y1p - cyp) / ry;
    a1 = nsvg__vecang(1.0f, 0.0f, ux, uy); // Initial angle
    da = nsvg__vecang(ux, uy, vx, vy);     // Delta angle

    //  if (vecrat(ux,uy,vx,vy) <= -1.0f) da = NSVG_PI;
    //  if (vecrat(ux,uy,vx,vy) >= 1.0f) da = 0;

    if (fa)
    {
        // Choose large arc
        if (da > 0.0f)
            da = da - 2 * NSVG_PI;
        else
            da = 2 * NSVG_PI + da;
    }

    // Approximate the arc using cubic spline segments.
    t[0] = cosrx;
    t[1] = sinrx;
    t[2] = -sinrx;
    t[3] = cosrx;
    t[4] = cx;
    t[5] = cy;

    // Split arc into max 90 degree segments.
    // The loop assumes an iteration per end point (including start and end), this
    // +1.
    ndivs = (int)(fabsf(da) / (NSVG_PI * 0.5f) + 1.0f);
    hda = (da / (float)ndivs) / 2.0f;
    kappa = fabsf(4.0f / 3.0f * (1.0f - cosf(hda)) / sinf(hda));
    if (da < 0.0f)
        kappa = -kappa;

    for (i = 0; i <= ndivs; i++)
    {
        a = a1 + da * (i / (float)ndivs);
        dx = cosf(a);
        dy = sinf(a);
        nsvg__xformPoint(&x, &y, dx * rx, dy * ry, t);                      // position
        nsvg__xformVec(&tanx, &tany, -dy * rx * kappa, dx * ry * kappa, t); // tangent
        if (i > 0)
            nsvg__cubicBezTo(p, px + ptanx, py + ptany, x - tanx, y - tany, x, y);
        px = x;
        py = y;
        ptanx = tanx;
        ptany = tany;
    }

    *cpx = x2;
    *cpy = y2;
}

static void nsvg__forceLowMemory(NSVGparser *p, char closedFlag)
{
    if (p->npts > 0)
    {
        nsvg__drawPath(p, closedFlag);
        p->pts[0] = p->pts[p->npts * 2 - 2];
        p->pts[1] = p->pts[p->npts * 2 - 1];
        p->npts = 1;
    }
}

static void nsvg__parsePathDescriptors(NSVGparser *p, FILE *fp, char quote)
{
    char cmd = '\0';
    float args[10];
    int nargs;
    int rargs = 0;
    float cpx, cpy, cpx2, cpy2;
    char closedFlag;
    char item[64];

    nsvg__resetPath(p);
    cpx = 0;
    cpy = 0;
    cpx2 = 0;
    cpy2 = 0;
    closedFlag = 0;
    nargs = 0;

    int s = fgetc(fp);

    while (s != EOF && s != '>' && s != quote)
    {
        item[0] = '\0';
        // Skip white spaces and commas
        while (s != EOF && s != '>' && s != quote && (nsvg__isspace(s) || s == ','))
            s = fgetc(fp);
        if (s == EOF || s == '>' || s == quote)
            break;
        if (s == '-' || s == '+' || s == '.' || nsvg__isdigit(s))
        {
            s = nsvg__parseNumber(fp, quote, s, item, 64);
        }
        else
        {
            // Parse command
            item[0] = s;
            item[1] = '\0';
            s = fgetc(fp);
        }
        if (!*item)
            break;
        if (nsvg__isnum(item[0]))
        {
            if (nargs < 10)
                args[nargs++] = (float)atof(item);
            if (nargs >= rargs)
            {
                switch (cmd)
                {
                case 'm':
                case 'M':
                    nsvg__pathMoveTo(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
                    // Moveto can be followed by multiple coordinate pairs,
                    // which should be treated as linetos.
                    cmd = (cmd == 'm') ? 'l' : 'L';
                    rargs = nsvg__getArgsPerElement(cmd);
                    cpx2 = cpx;
                    cpy2 = cpy;
                    break;
                case 'l':
                case 'L':
                    nsvg__pathLineTo(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
                    cpx2 = cpx;
                    cpy2 = cpy;
                    break;
                case 'H':
                case 'h':
                    nsvg__pathHLineTo(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
                    cpx2 = cpx;
                    cpy2 = cpy;
                    break;
                case 'V':
                case 'v':
                    nsvg__pathVLineTo(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
                    cpx2 = cpx;
                    cpy2 = cpy;
                    break;
                case 'C':
                case 'c':
                    nsvg__pathCubicBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'c' ? 1 : 0);
                    break;
                case 'S':
                case 's':
                    nsvg__pathCubicBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
                    break;
                case 'Q':
                case 'q':
                    nsvg__pathQuadBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'q' ? 1 : 0);
                    break;
                case 'T':
                case 't':
                    nsvg__pathQuadBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 't' ? 1 : 0);
                    break;
                case 'A':
                case 'a':
                    nsvg__pathArcTo(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
                    cpx2 = cpx;
                    cpy2 = cpy;
                    break;
                default:
                    if (nargs >= 2)
                    {
                        cpx = args[nargs - 2];
                        cpy = args[nargs - 1];
                        cpx2 = cpx;
                        cpy2 = cpy;
                    }
                    break;
                }
                nargs = 0;
            }
        }
        else
        {
            cmd = item[0];
            rargs = nsvg__getArgsPerElement(cmd);
            if (cmd == 'M' || cmd == 'm')
            {
                // Commit path.
                if (p->npts > 0)
                    nsvg__drawPath(p, closedFlag);
                // Start new subpath.
                nsvg__resetPath(p);
                closedFlag = 0;
                nargs = 0;
            }
            else if (cmd == 'Z' || cmd == 'z')
            {
                closedFlag = 1;
                // Commit path.
                if (p->npts > 0)
                {
                    // Move current point to first point
                    cpx = p->sx;
                    cpy = p->sy;
                    cpx2 = cpx;
                    cpy2 = cpy;
                    nsvg__drawPath(p, closedFlag);
                }
                // Start new subpath.
                nsvg__resetPath(p);
                nsvg__moveTo(p, cpx, cpy);
                closedFlag = 0;
                nargs = 0;
            }
            else
            {
                nsvg__forceLowMemory(p, closedFlag);
            }
        }
    }
    // Commit path.
    if (p->npts)
        nsvg__drawPath(p, closedFlag);
}

static void nsvg__parsePolyPoints(NSVGparser *p, FILE *fp, char quote)
{
    float args[2];
    int nargs, npts = 0;
    char item[64];
    int s = fgetc(fp);

    nargs = 0;
    while (s != EOF && s != '>' && s != quote)
    {
        item[0] = '\0';
        // Skip white spaces and commas
        while (s != EOF && s != '>' && s != quote && (nsvg__isspace(s) || s == ','))
            s = fgetc(fp);
        if (s == EOF || s == '>' || s == quote)
            break;
        if (s == '-' || s == '+' || s == '.' || nsvg__isdigit(s))
        {
            s = nsvg__parseNumber(fp, quote, s, item, 64);
        }
        else
        {
            // Parse command
            item[0] = s;
            item[1] = '\0';
            s = fgetc(fp);
        }
        args[nargs++] = (float)atof(item);
        if (nargs >= 2)
        {
            if (npts == 0)
                nsvg__moveTo(p, args[0], args[1]);
            else
                nsvg__lineTo(p, args[0], args[1]);
            nargs = 0;
            npts++;
        }
    }
}

static void nsvg__parseAttribs(NSVGparser *p, FILE *fp, int *end = NULL)
{
    memset(&p->attrs, 0, sizeof(NSVGCommonAttrs));
    /* Marks not set */
    p->attrs.rx = -1.0;
    p->attrs.rx = -1.0;
    static char attr_name[32];

    int s = fgetc(fp);

    // Get attribs
    while (!(end && *end) && s != '>' && s != EOF)
    {
        // Skip white space before the attrib name
        while (s != '>' && nsvg__isspace(s))
            s = fgetc(fp);
        if (s == '>' || s == EOF)
            return;
        if (s == '/')
        {
            if (end)
                *end = 1;
            return;
        }

        int attr_name_len = 0;
        // Find end of the attrib name.
        while (s != EOF && s != '>' && !nsvg__isspace(s) && s != '=' && attr_name_len < (32 - 2))
        {
            attr_name[attr_name_len++] = s;
            s = fgetc(fp);
        }
        attr_name[attr_name_len++] = '\0';

        // Skip until the beginning of the value.
        while (s != EOF && s != '>' && s != '\"' && s != '\'')
            s = fgetc(fp);
        if (s == '>')
            return;

        if (attr_name_len)
        {
            if (strcmp(attr_name, "d") == 0)
            {
                nsvg__parsePathDescriptors(p, fp, s);
            }
            else if (strcmp(attr_name, "points") == 0)
            {
                nsvg__parsePolyPoints(p, fp, s);
            }
            else if (strcmp(attr_name, "transform") == 0)
            {
                NSVGxform *xform = nsvg__getCurrentXForm(p);
                if (xform)
                {
                    float xform_floats[6];
                    nsvg__parseTransform(xform_floats, fp, s);
                    nsvg__xformPremultiply(xform->xform, xform_floats);
                }
            }
            else
            {
                if (strcmp(attr_name, "r") == 0)
                    p->attrs.r = nsvg__parseCoordinate(fp, s);
                else if (strcmp(attr_name, "width") == 0)
                    p->attrs.width = nsvg__parseCoordinate(fp, s);
                else if (strcmp(attr_name, "height") == 0)
                    p->attrs.height = nsvg__parseCoordinate(fp, s);
                else if (strcmp(attr_name, "x") == 0)
                    p->attrs.x = nsvg__parseCoordinate(fp, s);
                else if (strcmp(attr_name, "y") == 0)
                    p->attrs.y = nsvg__parseCoordinate(fp, s);
                else if (strcmp(attr_name, "rx") == 0)
                    p->attrs.rx = fabsf(nsvg__parseCoordinate(fp, s));
                else if (strcmp(attr_name, "ry") == 0)
                    p->attrs.ry = fabsf(nsvg__parseCoordinate(fp, s));
                else if (strcmp(attr_name, "cx") == 0)
                    p->attrs.cx = fabsf(nsvg__parseCoordinate(fp, s));
                else if (strcmp(attr_name, "cy") == 0)
                    p->attrs.cy = fabsf(nsvg__parseCoordinate(fp, s));
                else if (strcmp(attr_name, "x1") == 0)
                    p->attrs.x1 = nsvg__parseCoordinate(fp, s);
                else if (strcmp(attr_name, "y1") == 0)
                    p->attrs.y1 = nsvg__parseCoordinate(fp, s);
                else if (strcmp(attr_name, "x2") == 0)
                    p->attrs.x2 = nsvg__parseCoordinate(fp, s);
                else if (strcmp(attr_name, "y2") == 0)
                    p->attrs.y2 = nsvg__parseCoordinate(fp, s);
                else
                {
                    // Consume the attribute without saving it
                    char quote = s;
                    s = fgetc(fp);
                    while (s != EOF && s != '>' && s != quote)
                    {
                        s = fgetc(fp);
                    }
                }
            }
        }
        s = fgetc(fp);
    }
}

static void nsvg__parsePath(NSVGparser *p, FILE *input, int *is_end_tag)
{
    nsvg__parseAttribs(p, input, is_end_tag);
}

static void nsvg__parseRect(NSVGparser *p, FILE *input)
{
    nsvg__parseAttribs(p, input);

    float x = p->attrs.x;
    float y = p->attrs.y;
    float w = p->attrs.width;
    float h = p->attrs.height;
    float rx = p->attrs.rx;
    float ry = p->attrs.ry;

    if (rx < 0.0f && ry > 0.0f)
        rx = ry;
    if (ry < 0.0f && rx > 0.0f)
        ry = rx;
    if (rx < 0.0f)
        rx = 0.0f;
    if (ry < 0.0f)
        ry = 0.0f;
    if (rx > w / 2.0f)
        rx = w / 2.0f;
    if (ry > h / 2.0f)
        ry = h / 2.0f;

    if (w != 0.0f && h != 0.0f)
    {
        nsvg__resetPath(p);

        if (rx < 0.00001f || ry < 0.0001f)
        {
            nsvg__moveTo(p, x, y);
            nsvg__lineTo(p, x + w, y);
            nsvg__lineTo(p, x + w, y + h);
            nsvg__lineTo(p, x, y + h);
        }
        else
        {
            // Rounded rectangle
            nsvg__moveTo(p, x + rx, y);
            nsvg__lineTo(p, x + w - rx, y);
            nsvg__cubicBezTo(p, x + w - rx * (1 - NSVG_KAPPA90), y, x + w, y + ry * (1 - NSVG_KAPPA90), x + w, y + ry);
            nsvg__lineTo(p, x + w, y + h - ry);
            nsvg__cubicBezTo(p, x + w, y + h - ry * (1 - NSVG_KAPPA90), x + w - rx * (1 - NSVG_KAPPA90), y + h, x + w - rx, y + h);
            nsvg__lineTo(p, x + rx, y + h);
            nsvg__cubicBezTo(p, x + rx * (1 - NSVG_KAPPA90), y + h, x, y + h - ry * (1 - NSVG_KAPPA90), x, y + h - ry);
            nsvg__lineTo(p, x, y + ry);
            nsvg__cubicBezTo(p, x, y + ry * (1 - NSVG_KAPPA90), x + rx * (1 - NSVG_KAPPA90), y, x + rx, y);
        }

        nsvg__drawPath(p, 1);
    }
}

static void nsvg__parseCircle(NSVGparser *p, FILE *input)
{
    nsvg__parseAttribs(p, input);

    float cx = p->attrs.cx;
    float cy = p->attrs.cy;
    float r = p->attrs.r;

    if (r > 0.0f)
    {
        nsvg__resetPath(p);

        nsvg__moveTo(p, cx + r, cy);
        nsvg__cubicBezTo(p, cx + r, cy + r * NSVG_KAPPA90, cx + r * NSVG_KAPPA90, cy + r, cx, cy + r);
        nsvg__cubicBezTo(p, cx - r * NSVG_KAPPA90, cy + r, cx - r, cy + r * NSVG_KAPPA90, cx - r, cy);
        nsvg__cubicBezTo(p, cx - r, cy - r * NSVG_KAPPA90, cx - r * NSVG_KAPPA90, cy - r, cx, cy - r);
        nsvg__cubicBezTo(p, cx + r * NSVG_KAPPA90, cy - r, cx + r, cy - r * NSVG_KAPPA90, cx + r, cy);

        nsvg__drawPath(p, 1);
    }
}

static void nsvg__parseEllipse(NSVGparser *p, FILE *input)
{
    nsvg__parseAttribs(p, input);

    float cx = p->attrs.cx;
    float cy = p->attrs.cy;
    float rx = p->attrs.rx;
    float ry = p->attrs.ry;

    if (rx > 0.0f && ry > 0.0f)
    {

        nsvg__resetPath(p);

        nsvg__moveTo(p, cx + rx, cy);
        nsvg__cubicBezTo(p, cx + rx, cy + ry * NSVG_KAPPA90, cx + rx * NSVG_KAPPA90, cy + ry, cx, cy + ry);
        nsvg__cubicBezTo(p, cx - rx * NSVG_KAPPA90, cy + ry, cx - rx, cy + ry * NSVG_KAPPA90, cx - rx, cy);
        nsvg__cubicBezTo(p, cx - rx, cy - ry * NSVG_KAPPA90, cx - rx * NSVG_KAPPA90, cy - ry, cx, cy - ry);
        nsvg__cubicBezTo(p, cx + rx * NSVG_KAPPA90, cy - ry, cx + rx, cy - ry * NSVG_KAPPA90, cx + rx, cy);

        nsvg__drawPath(p, 1);
    }
}

static void nsvg__parseLine(NSVGparser *p, FILE *input)
{
    nsvg__parseAttribs(p, input);

    float x1 = p->attrs.x1;
    float y1 = p->attrs.y1;
    float x2 = p->attrs.x2;
    float y2 = p->attrs.y2;

    nsvg__resetPath(p);

    nsvg__moveTo(p, x1, y1);
    nsvg__lineTo(p, x2, y2);

    nsvg__drawPath(p, 0);
}

static void nsvg__parsePoly(NSVGparser *p, FILE *input, int closeFlag)
{
    nsvg__resetPath(p);
    nsvg__parseAttribs(p, input);
    nsvg__drawPath(p, (char)closeFlag);
}

static void nsvg__parseSVG(NSVGparser *p, FILE *input)
{
    nsvg__parseAttribs(p, input);
    p->width = p->attrs.width;
    p->height = p->attrs.height;
}

static void nsvg__parseGroup(NSVGparser *p, FILE *input, int *is_end_tag)
{
    nsvg__parseAttribs(p, input, is_end_tag);
}

static void nsvg__parseXML(FILE *fp, NSVGparser *p)
{
    int s = fgetc(fp);
    while (EOF != s)
    {
        if (s == '<')
        {
            s = fgetc(fp);

            // Skip white space after the '<'
            while (s != EOF && s != '<' && nsvg__isspace(s))
                s = fgetc(fp);

            // Start of a content or new tag.
            int start = 0;
            int end = 0;

            // Check if the tag is end tag
            if (s == '/')
            {
                s = fgetc(fp);
                end = 1;
            }
            else
            {
                start = 1;
            }

            // Skip comments, data and preprocessor stuff.
            if (s == EOF || s == '?' || s == '!')
                continue;

            // Get tag name
            static const int max_tag_name_len = 16;
            char tag_name[max_tag_name_len];
            int tag_name_len = 0;
            while (s != EOF && s != '<' && !nsvg__isspace(s) && tag_name_len < (max_tag_name_len - 2))
            {
                tag_name[tag_name_len++] = s;
                s = fgetc(fp);
            }
            tag_name[tag_name_len++] = '\0';

            if (start)
            {
                const char *known_tags[9] = { "g", "path", "rect", "circle", "ellipse", "line", "polyline", "polygon", "svg" };

                int found = 0;
                for (int i = 0; i < 9; i++)
                {
                    found += strcmp(tag_name, known_tags[i]) == 0;
                    if (found)
                        break;
                }
                if (!found)
                    continue;

                if (strcmp(tag_name, "g") == 0)
                {
                    nsvg__pushAttr(p);
                    nsvg__parseGroup(p, fp, &end);
                }
                else if (strcmp(tag_name, "path") == 0)
                {
                    if (p->pathFlag) // Do not allow nested paths.
                        continue;
                    nsvg__pushAttr(p);
                    nsvg__parsePath(p, fp, &end);
                    nsvg__popAttr(p);
                }
                else if (strcmp(tag_name, "rect") == 0)
                {
                    nsvg__pushAttr(p);
                    nsvg__parseRect(p, fp);
                    nsvg__popAttr(p);
                }
                else if (strcmp(tag_name, "circle") == 0)
                {
                    nsvg__pushAttr(p);
                    nsvg__parseCircle(p, fp);
                    nsvg__popAttr(p);
                }
                else if (strcmp(tag_name, "ellipse") == 0)
                {
                    nsvg__pushAttr(p);
                    nsvg__parseEllipse(p, fp);
                    nsvg__popAttr(p);
                }
                else if (strcmp(tag_name, "line") == 0)
                {
                    nsvg__pushAttr(p);
                    nsvg__parseLine(p, fp);
                    nsvg__popAttr(p);
                }
                else if (strcmp(tag_name, "polyline") == 0)
                {
                    nsvg__pushAttr(p);
                    nsvg__parsePoly(p, fp, 0);
                    nsvg__popAttr(p);
                }
                else if (strcmp(tag_name, "polygon") == 0)
                {
                    nsvg__pushAttr(p);
                    nsvg__parsePoly(p, fp, 1);
                    nsvg__popAttr(p);
                }
                else if (strcmp(tag_name, "svg") == 0)
                {
                    nsvg__parseSVG(p, fp);
                }
            }

            if (end)
            {
                if (strcmp(tag_name, "g") == 0)
                {
                    nsvg__popAttr(p);
                }
                else if (strcmp(tag_name, "path") == 0)
                {
                    p->pathFlag = 0;
                }
            }
        }
        s = fgetc(fp);

        while (s != EOF && s != '<')
            s = fgetc(fp);
    }
}

// Draws SVG from a file
void nsvgDrawFromFile(const char *filename, nsvgMoveToCb moveToCb, nsvgBeginPathCb beginPathCb, nsvgEndPathCb endPathCb, float scale,
                      float x0, float y0)
{
    FILE *fp = NULL;

    fp = fopen(filename, "rb");
    if (!fp)
        return;

    NSVGparser p = {};

    // Init style
    nsvg__xformIdentity(p.xforms[0].xform);

    p.moveToCb = moveToCb;
    p.beginPathCb = beginPathCb;
    p.endPathCb = endPathCb;
    p.scale = scale;
    p.x0 = x0;
    p.y0 = y0;

    nsvg__parseXML(fp, &p);

    p.endPathCb();

    free(p.pts);
    fclose(fp);
}
