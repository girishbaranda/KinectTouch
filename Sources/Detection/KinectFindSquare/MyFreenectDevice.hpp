#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <fstream>

class coords
{
public :
  coords(int _x, int _y, int _z, int _count):x(_x), y(_y), z(_z), counter(_count)
  {};
  ~coords(){};
  int x;
  int y;
  int z;
  int counter;
};

std::list<coords*> coordsList;
std::list<coords*> switchedTable;


class Mutex {
public:
  Mutex() {
    pthread_mutex_init( &m_mutex, NULL );
  }
  void lock() {
    pthread_mutex_lock( &m_mutex );
  }
  void unlock() {
    pthread_mutex_unlock( &m_mutex );
  }

  class ScopedLock
  {
    Mutex & _mutex;
  public:
    ScopedLock(Mutex & mutex)
      : _mutex(mutex)
    {
      _mutex.lock();
    }
    ~ScopedLock()
    {
      _mutex.unlock();
    }
  };
private:
  pthread_mutex_t m_mutex;
};

class MyFreenectDevice : public Freenect::FreenectDevice
{
public :
  MyFreenectDevice(freenect_context *_ctx, int _index)
    : Freenect::FreenectDevice(_ctx, _index), m_buffer_depth(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes),m_buffer_video(freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes), m_gamma(2048), m_new_rgb_frame(false), m_new_depth_frame(false)
  {
    for( unsigned int i = 0 ; i < 2048 ; i++) {
      float v = i/2048.0;
      v = std::pow(v, 3)* 6;
      m_gamma[i] = v*6*256;
    }
  }
  void VideoCallback(void* _rgb, __attribute__((unused))uint32_t timestamp) {
    Mutex::ScopedLock lock(m_rgb_mutex);
    rgb = static_cast<uint8_t*>(_rgb);

    m_new_rgb_frame = true;
  };

  void DepthCallback(void* _depth, uint32_t timestamp) {
    (void)timestamp;
    Mutex::ScopedLock lock(m_depth_mutex);
    depth = static_cast<uint16_t*>(_depth);
    int i = 0;
    std::list<coords*>::iterator it = switchedTable.begin();
    if (switchedTable.size() == 640 * 480)
      {
	while (i < 480)
	  {
	    int j = 0;
	    while (j < 640)
	      {
		(*it)->x = j;
		(*it)->y = i;
		(*it)->z = m_gamma[this->depth[i * 640 + j]];
		++it;
		++j;
	      }
	    ++i;
	  }
      }
    m_new_depth_frame = true;
  }

  void Initialize()
  {
    startDepth();
    startVideo();
    glClearDepth(1.f);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.f, 1.f, 1.f, 2500.f); 
    b = false;
    x = -250;
    y = 100;
    z = -1070;
    timeStamp = 0;
    int tmpX = 0, tmpY = 0;
    while (tmpY < 480)
      {
	tmpX = 0;
	while (tmpX < 640)
	  {
	    saved[tmpY][tmpX] = 0;
	    ++tmpX;
	  }
	++tmpY;
      }
  }

  bool detectEdges(int x, int y)
  {
    int matrixv[3];
    int matrixh[3];
    int toComputHor[3][3];
    int toComputVer[3][3];

    if (x > 1 && y > 1 && y < 480 && x < 640)
      {
	if (rgb[(y * 640 + x) * 3] > 5
	    && rgb[(y * 640 + x) * 3 + 1] > 5
	    && rgb[(y * 640 + x) * 3 + 2] > 5
	    && rgb[(y * 640 + x) * 3] < 100
	    && rgb[(y * 640 + x) * 3 + 1] < 100
	    && rgb[(y * 640 + x) * 3 + 2] < 100)
	  {
	    glPointSize(10);
	    glColor3f(1.f, 1.f, 1.f);
	    glVertex3f(x,y ,0);
	  }
	else if (rgb[(y * 640 + x) * 3] > 100
		 && rgb[(y * 640 + x) * 3 + 1] > 100
		 && rgb[(y * 640 + x) * 3 + 2] > 100)
	  {
	    if (rgb[(y * 640 + x) * 3] > 170
		&& rgb[(y * 640 + x) * 3 + 1] > 170
		&& rgb[(y * 640 + x) * 3 + 2] > 170)
	      {
		glPointSize(10);
		glColor3f(1.f, 0.f, 1.f);
		glVertex3f(x,y ,0);		
	      }
	    else
	      {
		glPointSize(10);
		glColor3f(0.f, 0.f, 0.f);
		glVertex3f(x,y ,0);
	      }
	  }
	else
	  {
	    glPointSize(10);
	    glColor3f(1.f, 0, 0);
	    glVertex3f(x,y ,0);
	  }
      }
  }

  bool checkAllMatrix(int x, int y)
  {
    std::list<int**>::iterator it = matrixList.begin();

    if (matrixList.size() > 300)
      {
	// b = true;
	matrixList.clear();
	timeStamp = 0;
      }
    if (timeStamp > 10)
      {
	int tmp = 0;
	
	while (it != matrixList.end())
	  {
	    if ((*it)[y][x] != surfaceMatrix[y][x])
	      {
		++tmp;
	      }
	    ++it;
	  }
	if (tmp > 5)
	  return false;
	
      }
    return true;
  }

  void affVideo()
  {
    int x = 0, y = 0;
    // int **tmp;

    // tmp = (int**)malloc(sizeof(int*) * 480);
    while (y < 480)
      {
	x = 0;
	// tmp[y] = (int*)malloc(sizeof(int) * 640);
	while (x < 640)
	  {
	    // tmp[y][x] = surfaceMatrix[y][x];
	    prevSurface[y][x] = surfaceMatrix[y][x];
	    surfaceMatrix[y][x] = 0;
	    ++x;
	  }
	++y;
      }
    // matrixList.push_back(tmp);
    x = 0, y = 0;
    if (m_new_rgb_frame == true)
      {
	glRotated(180, 1, 0, 0);
	glBegin(GL_POINTS);
	while (y < 480 * 3)
	  {
	    x = 0;
	    while (x < 640 * 3)
	      {
		unsigned char R = rgb[(y*640+x)+2];
		unsigned char G = rgb[(y*640+x)+1];
		unsigned char B = rgb[(y*640+x)];
		int gray = ceil(R * 0.3 + 0.59 * G + 0.11 * B);
		
		if (gray < 100)
		  gray = 0;
		if (gray > 200 && depth[(y*640 + x)] < 400)
		  surfaceMatrix[y/ 3][x/3] = 1;
		else
		  surfaceMatrix[y/3][x/3] = 0;
		x += 3;
	      }
	    y += 3;
	  }
	x = 0, y =0;
	int maxY = 0, maxX = 0, minX = 10000, minY = 10000;
	if (m_new_rgb_frame == true)
	  {
	    while (y < 480)
	      {
		x = 0;
		while (x < 640)
		  {
		    saved[y][x] = 0;
		    if (surfaceMatrix[y][x] == 1)
		      {
			if (surfaceMatrix[y - 1][x] == 0
			    && surfaceMatrix[y + 1][x] == 1)
			  {
			    if (y < minY)
			      minY = y;
			    saved[y][x] = 1;
			  }
			if (surfaceMatrix[y + 1][x] == 0
			    && surfaceMatrix[y - 1][x] == 1)
			  {
			    if (y > maxY)
			      maxY = y;
			    saved[y][x] = 1;
			  }
			if (surfaceMatrix[y][x - 1] == 0
			    && surfaceMatrix[y][x + 1] == 1)
			  {
			    if (minX > x)
			      minX = x;
			    saved[y][x] = 1;
			  }
			if (surfaceMatrix[y][x + 1] == 0
			    && surfaceMatrix[y][x - 1] == 1)
			  {
			    if (maxX < x)
			      maxX = x;
			    saved[y][x] = 1;
			  }
		      }
		    ++x;
		  }
		++y;
	      }
	    x = 0, y = 0;
	    while (y < 480)
	      {
		x = 0;
		while (x < 640)
		  {
		    if (saved[y][x] == 1)
		      {
			glColor3f(1.f, 0.f, 0.f);
			glVertex3f(x  * 3, y * 3, 0);
		      }
		    ++x;
		  }
		++y;
	      }
	  }
	// glPointSize(10);
	glColor3f(1, 1, 0);
	glVertex3f(maxX * 3, maxY * 3, 0);
	glVertex3f(maxX * 3, minY * 3, 0);
	glVertex3f(minX * 3, minY * 3, 0);
	glVertex3f(minX * 3, maxY * 3, 0);
	glEnd();
      }
    ++timeStamp;
  }
  
  void affCallback()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(x, y, z);
    glRotated(180, 1, 0, 0);
    glBegin(GL_POINTS);
    glPointSize(1.0);
    
    if (m_new_depth_frame && b == false)
      {
	int i = 0;
	int tmp = 0;
	while (i < 480)
	  {
	    int j = 0;
	    while (j < 640)
	      {
		int pval = m_gamma[depth[i * 640 + j]];
		coordsList.push_back(new coords(j, i, pval, tmp));
		switchedTable.push_back(new coords(j, i, pval, tmp));
		++j;
		++tmp;
	      }
	    ++tmp;
	    ++i;
	  }
	if (b == false)
	  b = true;
      }
    else if (m_new_depth_frame && b == true)
      {
	std::list<coords*>::iterator it;
	std::list<coords*>::iterator it2;
	std::list<coords*>::iterator itTmp;

	it = coordsList.begin();
	it2 = switchedTable.begin();
	while (it != coordsList.end())
	  {
	    glColor3ub(255, 255, 255);
	    if ((*it)->x == (*it2)->x &&
		(*it)->y == (*it2)->y)
	      {
		if ((*it2)->z < (*it)->z - 7 && (*it2)->z > (*it)->z - 10)
		  {
		    glColor3ub(0, 0, 0);
		    glVertex3f((*it2)->x, (*it2)->y, (*it2)->z);
		    float x, y;

		    if ((*it2)->x > (640/3)  && (*it2)->y > (480/3) * 2
			&& (*it2)->x < (640/3) * 2 && (*it2)->y < (480/3) * 3)
		      {
			fingerList.push_front(new coords((*it2)->x, (*it2)->y, (*it2)->z, 0));
		      }
		  }
		if ((*it)->x > (640/3)  && (*it)->y > (480/3) * 2
		    && (*it)->x < (640/3) * 2 && (*it)->y < (480/3) * 3)
		  glColor3ub(255, 0, 0);
		else
		  glColor3ub(255, 255, 255);
		glVertex3f((*it)->x, (*it)->y, (*it)->z);
	      }
	    ++it;
	    ++it2;
	  }
      }
    fingerList.clear();
    glEnd();
  }

public :
  std::vector<uint8_t> m_buffer_depth;
  std::vector<uint8_t> m_buffer_video;
  std::vector<uint16_t> m_gamma;
  Mutex m_rgb_mutex;
  Mutex m_depth_mutex;
  bool m_new_rgb_frame;
  bool m_new_depth_frame;
  uint16_t* depth;
  uint8_t* rgb;
  uint8_t* _image;
  bool b;
  std::list<coords*> fingerList;
  int surfaceMatrix[480][640];
  int prevSurface[480][640];
  int x, y, z;
  int timeStamp;
  int saved[480][640];
  std::list<int**> matrixList;
};
