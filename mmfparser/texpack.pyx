from libcpp.vector cimport vector
cdef extern from "<algorithm>" namespace "std":
    void sort[T, R](T first, T second, R comp)

cdef extern from "./maxrects/MaxRectsBinPack.cpp" namespace "rbp":
    cdef struct RectSize:
        int width
        int height

    cdef struct Rect:
        int x
        int y
        int width
        int height

    ctypedef enum FreeRectChoiceHeuristic:
        RectBestShortSideFit "rbp::MaxRectsBinPack::RectBestShortSideFit"
        RectBestLongSideFit "rbp::MaxRectsBinPack::RectBestLongSideFit"
        RectBestAreaFit "rbp::MaxRectsBinPack::RectBestAreaFit"
        RectBottomLeftRule "rbp::MaxRectsBinPack::RectBottomLeftRule"
        RectContactPointRule "rbp::MaxRectsBinPack::RectContactPointRule"

    cdef cppclass MaxRectsBinPack:
        void Init(int width, int height)
        bint Insert(const vector[RectSize] &rects, vector[Rect] &dst,
                    vector[int] &dstidx, FreeRectChoiceHeuristic method)

cdef extern from "./maxrects/Rect.cpp":
    pass


from PIL import Image

cdef class Sprite:
    cdef public:
        int x, y, w, h
        object image

    cdef void init(self, const Rect & rect, object image):
        self.x = rect.x
        self.y = rect.y
        self.w = rect.width
        self.h = rect.height
        self.image = image

cdef class MaxRects:
    cdef public:
        int width, height
        list results

    cdef void set_result(self, int width, int height,
                         vector[int] & idx, const vector[Rect] & dst,
                         list images):
        self.width = width
        self.height = height
        self.results = []
        cdef Sprite s

        cdef int rect_idx
        cdef int i = 0

        for rect_idx in idx:
            s = Sprite.__new__(Sprite)
            s.init(dst[i], images[rect_idx])
            self.results.append(s)
            i += 1

    def get(self):
        im = Image.new('RGBA', (self.width, self.height), (255, 20, 147, 255))

        cdef Sprite sprite
        for sprite in self.results:
            x, y, w, h = (sprite.x, sprite.y,
                          sprite.w, sprite.h)
            tmp = sprite.image.convert('RGBA')
            im.paste(tmp, (x, y, x + w, y + h))

        return im


cdef bint comp(int a, int b):
    return a > b


def pack_images(images, width, height):
    cdef list new_images = []
    cdef vector[RectSize] rects
    cdef Sprite sprite
    cdef RectSize rect

    for image in images:
        w, h = image.size
        if w > width or h > height:
            print 'Truncating image in texture atlas:', w, h
            image = image.crop((0, 0, min(w, width), min(h, height)))
        new_images.append(image)
        rect.width, rect.height = image.size
        rects.push_back(rect)

    cdef MaxRectsBinPack maxrects

    cdef vector[int] idx
    cdef vector[Rect] dst

    cdef MaxRects res = MaxRects()
    cdef int i

    while new_images:
        maxrects.Init(width, height)
        maxrects.Insert(rects, dst, idx, RectBestShortSideFit)
        res.set_result(width, height, idx, dst, new_images)
        sort(idx.begin(), idx.end(), comp)

        for i in idx:
            rects.erase(rects.begin() + i)
            del new_images[i]

        yield res

        print 'remaining sprites:', len(new_images)
