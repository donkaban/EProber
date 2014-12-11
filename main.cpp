#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <iostream>
#include <stdlib.h>
#include <cstring>

uint _width  = 0;
uint _height = 0;

Display * display = NULL;
Window    window;
Window    root;    
int       screen;
Visual  * visual = NULL; 

EGLNativeWindowType egl_window;
EGLDisplay          egl_display;
EGLContext          egl_context;
EGLSurface          egl_surface;
EGLConfig           egl_config;

float pos_x = 0.5f;
float pos_y = 0.5f;

void _checkEGL()  
{
    int e = eglGetError();
    if(e !=EGL_SUCCESS) 
    {
        std::cout << "\nEGL ERROR : " << e << std::endl;
        exit(-1);
    }
}    
void _checkGL()
{
    int e = glGetError();
    if(e !=GL_NO_ERROR)
    {
        std::cout << "\nGL ERROR : " << e << std::endl;
        exit(-1);
    }    
}

int getInt(const GLenum &e) {GLint val = 0; glGetIntegerv(e, &val); return val;}
#define GET_INT(e) {std::cout << #e  << ": " << getInt(e) << std::endl;} 
#define GET_STR(e) {std::cout << #e  << ": " << glGetString(e) << std::endl;} 

void _getGLInfo()
{
    GET_STR(GL_VENDOR); 
    GET_STR(GL_VERSION); 
    GET_STR(GL_RENDERER);  
    GET_STR(GL_SHADING_LANGUAGE_VERSION); 
    GET_INT(GL_SHADER_COMPILER);
    GET_INT(GL_ALPHA_BITS);
    GET_INT(GL_DEPTH_BITS);
    GET_INT(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
    GET_INT(GL_MAX_TEXTURE_IMAGE_UNITS);
    GET_INT(GL_MAX_TEXTURE_SIZE);
    GET_INT(GL_MAX_CUBE_MAP_TEXTURE_SIZE);
    GET_INT(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
    GET_INT(GL_NUM_SHADER_BINARY_FORMATS);
    GET_INT(GL_PACK_ALIGNMENT);
    GET_INT(GL_SAMPLE_BUFFERS);
    GET_INT(GL_MAX_RENDERBUFFER_SIZE);
    GET_INT(GL_MAX_VARYING_VECTORS); 
    GET_INT(GL_MAX_VERTEX_ATTRIBS);
    GET_INT(GL_MAX_VERTEX_UNIFORM_VECTORS);
    GET_INT(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS);
    GET_INT(GL_MAX_VIEWPORT_DIMS);
   std::cout << "Ext:\n" << glGetString(GL_EXTENSIONS) << std::endl; 
}

#define FATAL(msg) {std::cout << "\nERROR : " << msg << std::endl; exit(-1);}
#define LOG(msg)   std::cout << msg;
#define DONE  std::cout << "DONE" << std::endl;


Bool waitMapping(Display *, XEvent *e, char *arg )
{
    LOG("WAIT FOR MAPPING ")
    return (e->type == MapNotify) && (e->xmap.window == (Window)arg);
}


void initX11_mini()
{
    LOG("Init X11 system ... ")
    XEvent                  event;
   
    display = XOpenDisplay(NULL);
    if(!display) FATAL("can't open X11 display");
    
    root   = XDefaultRootWindow(display);
    screen = XDefaultScreen(display);
    visual = XDefaultVisual(display, screen);
    window = XCreateSimpleWindow(display,root,0,0,_width, _height,0,0,0);

    XSetWindowAttributes attr; 
    std::memset(&attr,0,sizeof(attr));
    attr.event_mask = StructureNotifyMask|ButtonPressMask|ButtonReleaseMask|ButtonMotionMask;
    attr.override_redirect  = True;
    attr.background_pixel   = 0xFFFFFF00;
    XWithdrawWindow(display,window, screen);  
    XChangeWindowAttributes(display,window,CWBackPixel|CWOverrideRedirect|CWSaveUnder|CWEventMask|CWBorderPixel, &attr);
    XMapWindow(display,window);
    XIfEvent(display, &event, waitMapping, (char*)window );
    XSetInputFocus(display,window, RevertToNone, CurrentTime);
    XFlush(display);
    DONE

}
void initGL_es()
{
    LOG("Init EGL ... ")
   
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,EGL_NONE, EGL_NONE };
    
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;

    EGLint attr[] =
    {
        EGL_RED_SIZE,           5,
        EGL_GREEN_SIZE,         6,
        EGL_BLUE_SIZE,          5,
        EGL_DEPTH_SIZE,         8,
        EGL_ALPHA_SIZE,         EGL_DONT_CARE,
        EGL_STENCIL_SIZE,       EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS,     EGL_DONT_CARE,
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT, 
        EGL_NONE
    };
    egl_display   = eglGetDisplay((EGLNativeDisplayType)display); 
    egl_window    = (EGLNativeWindowType) window;
    if(
        (!eglInitialize(egl_display, &majorVersion, &minorVersion)) || 
        (!eglBindAPI(EGL_OPENGL_ES_API))                            ||
        (!eglGetConfigs(egl_display,0,0,&numConfigs))               || 
        (!eglChooseConfig(egl_display, attr, &egl_config, 1, &numConfigs)))
        _checkEGL(); 
    egl_surface = eglCreateWindowSurface(egl_display, egl_config, egl_window, NULL);         _checkEGL();
    egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, contextAttribs); _checkEGL();
    if(!eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context))                  _checkEGL();
}
void close()
{
    LOG("Close all ...")
    eglDestroyContext(egl_display, egl_context);
    XDestroyWindow(display,window);
    if(visual) delete visual;
}


void init(uint w, uint h)
{
    LOG("Init...\n")
    _width = w;
    _height = h;
    initX11_mini();
    initGL_es(); 
    glViewport(0, 0, static_cast<GLsizei>(_width), static_cast<GLsizei>(_height));
    glClearColor(.3, .3, .5, 0);
    _getGLInfo();
}

void draw();

bool done = false;
void pollX11events()
{
        XEvent evt;
        for (int i = 0; i < XPending(display); i++)
        { 
            XNextEvent(display, &evt);
            switch (evt.type)
            {
                case ButtonPress:
                {
                    LOG("event : touch! \n");
                    XButtonEvent * event = (XButtonEvent *) &evt;
                    pos_x = ((float)event->x/_width) ;
                    pos_y = ((float)event->y/_height);
                    break;
                }
                case MotionNotify:
                {
                    LOG("event : move! \n");
                    XMotionEvent * event = (XMotionEvent *) &evt;
                    pos_x = ((float)event->x/_width);
                    pos_y = ((float)event->y/_height);
                    std::cout << "pos: " << pos_x << ", " << pos_y << "\n";
                    break;
                }
                case ButtonRelease :
                    LOG("event : release! \n");
          
                default:
                    break;
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLuint id[2];
GLuint mat;

const float vertex[] = 
{
  -1.0f, 1.0f, 0.0f, 1.0f    , 1.0f, 1.0f, 0.0f, 0.0f,
  -1.0f,-1.0f, 0.0f, 1.0f    , 1.0f, 0.0f, 0.0f, 0.0f,
   1.0f,-1.0f, 0.0f, 1.0f    , 0.0f, 0.0f, 0.0f, 0.0f,
   1.0f, 1.0f, 0.0f, 1.0f    , 0.0f, 1.0f, 0.0f, 0.0f
};
const unsigned char ndx[] = {0,1,2,2,3,0};
const char *v_src = "precision mediump float;attribute vec4 pos;attribute vec2 uva;varying vec2 uv;void main(){uv = uva;gl_Position = pos;}";
const char *f_src = "precision mediump float;uniform float t;uniform vec2 p;varying vec2 uv;void main(){float x = uv.x + p.x - 1.0;float y = uv.y + p.y - 1.0;float r = (x * x + y * y);float z = cos((r +  t * 0.2)/0.01);gl_FragColor = vec4(z,z,z,1);}";
GLuint t,p;

void init_draw()
{
    LOG("Init Draw ... ")
	glGenBuffers(2, id);
	glBindBuffer(GL_ARRAY_BUFFER, id[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,id[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex),vertex,GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ndx), ndx,GL_STATIC_DRAW);
	GLint v_shader = glCreateShader(GL_VERTEX_SHADER);    
	GLint f_shader = glCreateShader(GL_FRAGMENT_SHADER);  
	glShaderSource(v_shader, 1, &v_src, NULL); 
	glShaderSource(f_shader, 1, &f_src, NULL);
	glCompileShader(v_shader); 
	glCompileShader(f_shader); 
	mat = glCreateProgram(); 
	glAttachShader(mat, v_shader);
	glAttachShader(mat, f_shader);
	glLinkProgram(mat);
    
    glBindBuffer(GL_ARRAY_BUFFER,id[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,id[1]);
    glVertexAttribPointer(glGetAttribLocation(mat,"pos"), 4 ,GL_FLOAT, GL_FALSE, sizeof(vertex) / 4 , (const void *) 0);
    glVertexAttribPointer(glGetAttribLocation(mat,"uva"), 4, GL_FLOAT, GL_FALSE, sizeof(vertex) / 4,  (const void *) (sizeof(float) * 4));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
 
    glUseProgram(mat);
    t = glGetUniformLocation(mat,"t");
    p = glGetUniformLocation(mat,"p");
 }

float fake_time = 0.0;
void draw()
{
    fake_time += 0.01;
    glUniform1f(t, fake_time);
    glUniform2f(p, pos_x, pos_y);
    glDrawElements(GL_TRIANGLES,sizeof(ndx)/sizeof(unsigned char) ,GL_UNSIGNED_BYTE,(const void *) 0);  
   _checkGL();
}

void update()
{
    pollX11events();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    draw();
    eglSwapBuffers(egl_display, egl_surface);
    _checkGL();
   
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
    
    init(800,480);
    init_draw();
    while(!done)
        update();
    close();
    return 0;
}