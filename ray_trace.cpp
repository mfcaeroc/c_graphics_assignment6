//---------------------------------------
// Program: ray_trace.cpp
// Purpose: Demonstrate ray tracing.
// Author:  John Gauch
// Date:    Spring 2019
//---------------------------------------
#include <cmath>
#include <cstdio>
#include <cstdlib>
#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
using namespace std;

// Include ray tracing and phong shading classes
#include "ray_classes.h"

// Global variables
#define XDIM 600
#define YDIM 600
#define ZDIM 600
unsigned char image[YDIM][XDIM][3];
float position = -5;
string mode = "phong";
float Bounce = -1;
const float RADIUS = 2.0;
const int SPHERES = 10;
Sphere3D sphere[SPHERES];
ColorRGB color[SPHERES];

//---------------------------------------
// Calculate random value between [min..max]
//---------------------------------------
float myrand(float min, float max)
{
   return rand() * (max - min) / RAND_MAX + min;
}

//---------------------------------------
// Check to see if point is in shadow
//---------------------------------------
bool in_shadow(Point3D pt, Vector3D dir, int current, Sphere3D sphere[], int count)
{
   // Define ray to light source
   Ray3D shadow_ray;
   shadow_ray.set(pt, dir);

   // Check to see ray intersects any sphere
   Point3D point;
   Vector3D normal;
   for (int index=0; index<count; index++)
      if ((index != current) && 
         (sphere[index].get_intersection(shadow_ray, point, normal)))
         return true;
   return false;
}

//---------------------------------------
// Perform ray tracing of scene
//---------------------------------------
void ray_trace()
{
   // Define camera point
   Point3D camera;
   camera.set(0,0,position);

   // Define light source
   ColorRGB light_color;
   light_color.set(250,250,250);
   Vector3D light_dir;
   light_dir.set(-1,-1,-1);
   light_dir.normalize();

   // Define shader
   Phong shader;
   shader.SetCamera(camera);
   shader.SetLight(light_color, light_dir);

   // Perform ray tracing
   for (int y = 0; y < YDIM; y++)
   for (int x = 0; x < XDIM; x++)
   {
      // Clear image
      image[y][x][0] = 0;
      image[y][x][1] = 0;
      image[y][x][2] = 0;

      // Define sample point on image plane
      float xpos = (x - XDIM/2) * 2.0 / XDIM;
      float ypos = (y - YDIM/2) * 2.0 / YDIM;
      Point3D point;
      point.set(xpos, ypos, 0);
   
      // Define ray from camera through image
      Ray3D ray;
      ray.set(camera, point);

      // Perform sphere intersection
      int closest = -1;
      Point3D p, closest_p;
      Vector3D n, closest_n;
      closest_p.set(0,0,ZDIM);
      for (int s=0; s<SPHERES; s++)
      {
         if ((sphere[s].get_intersection(ray, p, n)) && (p.pz < closest_p.pz))
         {
            closest = s;
            closest_p = p;
            closest_n = n;
         }
      }

      // Calculate pixel color
      if (closest >= 0)
      {
         // Display surface normal
         if (mode == "normal")
         {
            image[y][x][0] = 127 + closest_n.vx * 127;
            image[y][x][1] = 127 + closest_n.vy * 127;
            image[y][x][2] = 127 + closest_n.vz * 127;
         }

         // Calculate Phong shade
         if (mode == "phong")
         {
            // Check to see if in shadow 
            if (in_shadow(closest_p, light_dir, closest, sphere, SPHERES))
               shader.SetObject(color[closest], 0.4, 0.0, 0.0, 1);
            else
               shader.SetObject(color[closest], 0.4, 0.4, 0.4, 10);

      
            // Calculate pixel color
            ColorRGB pixel;
            shader.GetShade(closest_p, closest_n, pixel);
            image[y][x][0] = pixel.R;
            image[y][x][1] = pixel.G;
            image[y][x][2] = pixel.B;
         }
      }
   }
}
 
//---------------------------------------
// Init function for OpenGL
//---------------------------------------
void init()
{
   // Initialize OpenGL
   glClearColor(0.0, 0.0, 0.0, 1.0);

   // Print command menu
   cout << "Program commands:\n"
        << "   '+' - increase camera distance\n"
        << "   '-' - decrease camera distance\n"
        << "   'p' - show Phong shading\n"
        << "   'n' - show surface normals\n"
        << "   'q' - quit program\n";

   // Define array of spheres
   srand(time(NULL));
   for (int s=0; s<SPHERES; s++)
   {
      float cx = myrand(-RADIUS/2, RADIUS/2);
      float cy = myrand(-RADIUS/2, RADIUS/2);
      float cz = myrand(0, RADIUS/2);
      Point3D center;
      center.set(cx,cy,cz);

      float mx = myrand(-RADIUS/100, RADIUS/200);
      float my = myrand(-RADIUS/100, RADIUS/200);
      float mz = myrand(-RADIUS/100, RADIUS/200);
      Vector3D motion;
      motion.set(mx,my,mz);
      float radius = myrand(RADIUS/20, RADIUS/10);
      sphere[s].set(center, motion, radius);
      int R = rand() % 255;
      int G = rand() % 255;
      int B = rand() % 255;
      color[s].set(R,G,B);
   }

   // Perform ray tracing
   cout << "camera: 0,0," << position << endl;
   ray_trace();
}

//---------------------------------------
// Display callback for OpenGL
//---------------------------------------
void display()
{
   // Display image
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(XDIM, YDIM, GL_RGB, GL_UNSIGNED_BYTE, image);
   glFlush();
}

//---------------------------------------
// Keyboard callback for OpenGL
//---------------------------------------
void keyboard(unsigned char key, int x, int y)
{
   // End program
   if (key == 'q')
      exit(0);

   // Move camera position
   else if (key == '+' && position > -10)
   {
      position = position - 0.5;
      cout << "camera: 0,0," << position << endl;
   }
   else if (key == '-' && position < -5)
   {
      position = position + 0.5;
      cout << "camera: 0,0," << position << endl;
   }

   // Change display mode
   else if (key == 'n')
      mode = "normal";
   else if (key == 'p')
      mode = "phong";

   // Perform ray tracing
   ray_trace();
   glutPostRedisplay();
}

//---------------------------------------
// Timer callback for OpenGL
//---------------------------------------
void timer(int value)
{
   // Move bouncing balls
   int i;
   for (i = 0; i < SPHERES; i++)
   {
      // Update ball position
      sphere[i].center.px += sphere[i].motion.vx;
      sphere[i].center.py += sphere[i].motion.vy;
      sphere[i].center.pz += sphere[i].motion.vz;

      // Bounce off walls
      if (sphere[i].center.px > RADIUS/2 - sphere[i].radius) 
         {sphere[i].center.px = RADIUS/2 - sphere[i].radius; 
          sphere[i].motion.vx *= Bounce; }
      if (sphere[i].center.py > RADIUS/2 - sphere[i].radius) 
         {sphere[i].center.py = RADIUS/2 - sphere[i].radius; 
          sphere[i].motion.vy *= Bounce; }
      if (sphere[i].center.pz > RADIUS/2 - sphere[i].radius) 
         {sphere[i].center.pz = RADIUS/2 - sphere[i].radius; 
          sphere[i].motion.vz *= Bounce; }
      if (sphere[i].center.px < -RADIUS/2 + sphere[i].radius) 
         {sphere[i].center.px = -RADIUS/2 + sphere[i].radius; 
          sphere[i].motion.vx *= Bounce; }
      if (sphere[i].center.py < -RADIUS/2 + sphere[i].radius) 
         {sphere[i].center.py = -RADIUS/2 + sphere[i].radius; 
          sphere[i].motion.vy *= Bounce; }
      if (sphere[i].center.pz < -RADIUS/2 + sphere[i].radius) 
         {sphere[i].center.pz = -RADIUS/2 + sphere[i].radius; 
          sphere[i].motion.vz *= Bounce; }

   }

   // Calculate and display image
   ray_trace();
   glutPostRedisplay();
   glutTimerFunc(10, timer, 0);
}


//---------------------------------------
// Main program
//---------------------------------------
int main(int argc, char *argv[])
{
   // Create OpenGL window
   glutInit(&argc, argv);
   glutInitWindowSize(XDIM, YDIM);
   glutInitWindowPosition(0, 0);
   glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
   glutCreateWindow("Ray Trace");
   init();

   // Specify callback function
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutTimerFunc(10, timer, 0);
   glutMainLoop();
   return 0;
}
