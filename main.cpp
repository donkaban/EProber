#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <stdexcept>
#include <iostream>
#include <unordered_map>
#include <chrono>
#include <sstream>

#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <thread>

#define SIZE_ASSERT(T, size) static_assert((sizeof(T) == size),"static alert : size mismatch!")

typedef signed   char       int8;    SIZE_ASSERT(int8,  1);
typedef unsigned char       uint8;   SIZE_ASSERT(uint8, 1);
typedef signed   short int  int16;   SIZE_ASSERT(int16, 2);
typedef unsigned short int  uint16;  SIZE_ASSERT(uint16,2);
typedef signed   int  		int32;   SIZE_ASSERT(int32, 4);
typedef unsigned int  		uint32;  SIZE_ASSERT(uint32,4);


uint _width  = 0;
uint _height = 0;

Display * display = NULL;
Window    window;
Window    root;    
GC        X11GC;
int       screen;
Visual  * visual = NULL; 

EGLNativeWindowType egl_window;
EGLDisplay          egl_display;
EGLContext          egl_context;
EGLSurface          egl_surface;
EGLConfig           egl_config;

std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

GLuint t,c1,c2;
float col1 = 0.5f;
float col2 = 0.5f;


void _checkEGL()  
{
    auto e = eglGetError();
    if(e !=EGL_SUCCESS) 
    {
        std::cout << "\nEGL ERROR : " << e << std::endl;
        exit(-1);
    }
}    
void _checkGL()
{
    auto e = glGetError();
    if(e !=GL_NO_ERROR)
    {
        std::cout << "\nGL ERROR : " << e << std::endl;
        exit(-1);
    }    
}

std::string _getGLInfo()
{
  return std::string(  
    "\nEGL      : " + std::string(eglQueryString(egl_display,EGL_VENDOR)) + 
    "\nVersion  : " + std::string((const char *)glGetString(GL_VERSION)) + 
    "\nRenderer : " + std::string((const char *)glGetString(GL_RENDERER)) + 
    "\nShaders  : " + std::string((const char *)glGetString(GL_SHADING_LANGUAGE_VERSION)) + 
    "\nEXT      : " + std::string((const char *)glGetString(GL_EXTENSIONS)) + 
    "\n\n");    
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
    X11GC = XCreateGC(display,window,0,0);
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
    egl_display   = eglGetDisplay(EGL_DEFAULT_DISPLAY); 
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
    DONE
 
}
void close()
{
    LOG("Close all ...")
    eglDestroyContext(egl_display, egl_context);
    XDestroyWindow(display,window);
    if(visual) delete visual;
    //if(display) XCloseDisplay(display);
    DONE
}





void init(uint w, uint h)
{
    LOG("Init...\n")
    _width = w;
    _height = h;
    initX11_mini();
    initGL_es(); 
    glViewport(0, 0, static_cast<GLsizei>(_width), static_cast<GLsizei>(_height));
    glClearColor(.2, .2, .5, 0);
    LOG(_getGLInfo());
}


void draw();

std::chrono::milliseconds sleeptick(10);
bool done = false;


void pollX11events()
{
        LOG("   POLL X11\n")
        XEvent evt;
        auto pending =  XPending(display);
        if(pending > 0)  LOG("      EVT:  ") 
        for (int i = 0; i < pending; i++)
        { 
            XNextEvent(display, &evt);
            switch (evt.type)
            {
                case ButtonPress:
                    LOG("+ press! ");
                    
                    break;
                case MotionNotify:
                {
                    LOG("+ move! ");
                    auto event = (XMotionEvent *) &evt;
                    col1 = (float)event->x/_width;
                    col2 = (float)event->y/_height;
          
                    break;
                }
                case ButtonRelease :
                    LOG("+ release! ");
          
                default:
                    break;
            }
        }
        LOG("   POLL X11 SYNC\n")
        XSync(display, True);
        LOG("   POLL X11 DONE\n")
    }


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const float vertex[] = 
{
 // x      y    z     w         s     t       u     v
  -1.0f, 1.0f, 0.0f, 1.0f    , 1.0f, 1.0f, 0.0f, 0.0f,
  -1.0f,-1.0f, 0.0f, 1.0f    , 1.0f, 0.0f, 0.0f, 0.0f,
   1.0f,-1.0f, 0.0f, 1.0f    , 0.0f, 0.0f, 0.0f, 0.0f,
   1.0f, 1.0f, 0.0f, 1.0f    , 0.0f, 1.0f, 0.0f, 0.0f
};

static const uint8_t ndx[] = {0,1,2,2,3,0};
GLuint id[2];
GLuint mat;
const char *v_src = R"(
    precision mediump float;
    attribute vec4  position; 
    attribute vec2  texcoord; 
    varying   vec2  v_texcoord; 
    void main() 
    {
        v_texcoord = texcoord; 
        gl_Position = position;
    })";


const char *f_src = R"(
    precision mediump float;
    varying vec2 v_texcoord;
    uniform float time;
    uniform float color1;
    uniform float color2;
    
	void main( void ) 
	{
		vec2 center = vec2(.5,.5);    
   		float x = 0.5 - v_texcoord.x;
    	float y = 0.5 - v_texcoord.y;
    	float r = (x * x + y * y);
    	float z = cos((r +  time * 0.2)/0.01);
    	gl_FragColor = vec4(color1*z,color2*z,z,1);
	}	

	

    )";

void init_draw()
{
    LOG("Init Draw ... ")
	glGenBuffers(2, id);
	glBindBuffer(GL_ARRAY_BUFFER, id[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,id[1]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex),vertex,GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ndx), ndx,GL_STATIC_DRAW);
	auto v_shader = glCreateShader(GL_VERTEX_SHADER);    
	auto f_shader = glCreateShader(GL_FRAGMENT_SHADER);  
	glShaderSource(v_shader, 1, &v_src, NULL); 
	glShaderSource(f_shader, 1, &f_src, NULL);
	glCompileShader(v_shader); 
	glCompileShader(f_shader); 
	mat = glCreateProgram(); 
	glAttachShader(mat, v_shader);
	glAttachShader(mat, f_shader);
	glLinkProgram(mat);
    
    DONE
   
}

static const auto SIZE = sizeof(ndx)/sizeof(uint8_t);

void draw()
{
    LOG("      SETUP\n");

    glUseProgram(mat);
    t = glGetUniformLocation(mat,"time");
    c1 = glGetUniformLocation(mat,"color1");
    c2 = glGetUniformLocation(mat,"color2");
    glBindBuffer(GL_ARRAY_BUFFER,id[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,id[1]);
    glVertexAttribPointer(glGetAttribLocation(mat,"position"), 4 ,GL_FLOAT, GL_FALSE, sizeof(vertex) / 4 , (const void *) 0);
    glVertexAttribPointer(glGetAttribLocation(mat,"texcoord"), 4, GL_FLOAT, GL_FALSE, sizeof(vertex) / 4,  (const void *) (sizeof(float) * 4));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
   
    _checkGL();

    LOG("      GET TIME\n");
    std::chrono::duration<float> dt = std::chrono::system_clock::now() - start;
    LOG("      SET UNIFORMS\n");
    glUniform1f(t, fmod(dt.count(),3.14));
    glUniform1f(c1, col1);
    glUniform1f(c2, col2);
    LOG("      DRAWELEMENTS ") LOG(SIZE) LOG(" \n")
    glFlush();
    glFinish();
    glDrawElements(GL_TRIANGLES,SIZE ,GL_UNSIGNED_BYTE,(const void *) 0);  
    glFlush();
    glFinish();

    LOG("      DRAWELEMENTS DONE\n");
   _checkGL();
 
   
}

void update()
{
    long i = 0;
    while(!done)
    {   
        LOG("FRAME")
        std::chrono::duration<float> dt = std::chrono::system_clock::now() - start;
        LOG(" # ") LOG (i++) LOG(" TIME: ") LOG (dt.count()) LOG (" sec.\n") 
        pollX11events();
        
        LOG("   CLEAR\n")
        glClear(GL_COLOR_BUFFER_BIT);
        LOG("   DRAW\n") 
        if(!glIsProgram(mat))  LOG("+++++++++++++++++++++ PROGRAMM DROPPED ERROR! +++++++++++++++++++++++\n");
        draw();
        LOG("   SWAP") LOG(std::endl)
        if(eglSwapBuffers(egl_display, egl_surface) == EGL_FALSE)
            LOG("+++++++++++++++++++++ SWAP ERROR! +++++++++++++++++++++++\n");
        _checkEGL();
        LOG("OK\n")
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main()
{
    
	   init(800,480);
	   init_draw();
 	   update();
       
    return 0;
}