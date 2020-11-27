/* compiles with command line  gcc xlibdemo.c -lX11 -lm -L/usr/X11R6/lib */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <bits/stdc++.h>
#include <vector>
#include "cell.h"
#include <limits.h>
#include <float.h>
#include <bits/stdc++.h>
#include <algorithm>

Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Example Window";
char *icon_name_string = "Icon for Example Window";

XEvent report;

GC gc, gc_yellow, gc_red, gc_grey;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

int zoopath(int a[][4], int b, int c[][2], int d);

void find_edges(Vertex src, Vertex dest, vector<Edge> &edges, vector<Vertex> vertexes);

int find_valid_edges(Edge e1, Edge e2);

int Dijkastra(Vertex src, Vertex dest, vector<Vertex> vertexes);

void Prim(int b, int* matrix, vector<Edge> &sequence);

void run_dfs(int v, bool visited[], list<int>* adj, vector<int> &index);

void find_nonblock_line(Vertex src, Vertex dest, vector<Edge> &edges, vector<Vertex> vertexes);

void find_block_id(Vertex src, Vertex dest, vector<Vertex> vertexes, vector<int> &id);


int main(int argc, char **argv)
{ int rectangles[200][4];
  int path[1000][2];
  int pathlength, rectanglenumber;
  int i;

  // record distance
  //int Distance[500][500];
  // record whether visited
  //int visited[500][500];

  /* opening display: basic connection to X Server */
  if( argc == 2 )
  {  FILE *input;
     input = fopen( argv[1], "r");
     if( input != NULL )
       printf("opened input file %s\n", argv[1] );
     else
       {  printf("cannot open input file %s\n", argv[1] );
	  exit( 0 );
       }
     i=0;
     while( fscanf( input, "[%d,%d]x[%d,%d]\n",
		    &(rectangles[i][0]), &(rectangles[i][1]),
		    &(rectangles[i][2]), &(rectangles[i][3]) ) == 4 )
       i+=1;
     printf("found %d rectangles in file %s\n", i, argv[1] );
     rectanglenumber = i; pathlength = 0;
  }
  if( (display_ptr = XOpenDisplay(display_name)) == NULL )
    { printf("Could not open display. \n"); exit(-1);}
  printf("Connected to X server  %s\n", XDisplayName(display_name) );
  screen_num = DefaultScreen( display_ptr );
  screen_ptr = DefaultScreenOfDisplay( display_ptr );
  color_map  = XDefaultColormap( display_ptr, screen_num );
  display_width  = DisplayWidth( display_ptr, screen_num );
  display_height = DisplayHeight( display_ptr, screen_num );

  printf("Width %d, Height %d, Screen Number %d\n", 
           display_width, display_height, screen_num);
  
  /* creating the window */
  border_width = 10;
  win_x = 0; win_y = 0;
  win_width = display_width/2;
  win_height = (int) (win_width / 1.7); /*rectangular window*/
  
  win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                            win_x, win_y, win_width, win_height, border_width,
                            BlackPixel(display_ptr, screen_num),
                            WhitePixel(display_ptr, screen_num) );
  /* now try to put it on screen, this needs cooperation of window manager */
  size_hints = XAllocSizeHints();
  wm_hints = XAllocWMHints();
  class_hints = XAllocClassHint();
  if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
    { printf("Error allocating memory for hints. \n"); exit(-1);}

  size_hints -> flags = PPosition | PSize | PMinSize  ;
  size_hints -> min_width = 60;
  size_hints -> min_height = 60;

  XStringListToTextProperty( &win_name_string,1,&win_name);
  XStringListToTextProperty( &icon_name_string,1,&icon_name);
  
  wm_hints -> flags = StateHint | InputHint ;
  wm_hints -> initial_state = NormalState;
  wm_hints -> input = False;

  class_hints -> res_name = "x_use_example";
  class_hints -> res_class = "examples";

  XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                    size_hints, wm_hints, class_hints );

  /* what events do we want to receive */
  XSelectInput( display_ptr, win, 
            ExposureMask | StructureNotifyMask | ButtonPressMask );
  
  /* finally: put window on screen */
  XMapWindow( display_ptr, win );

  XFlush(display_ptr);

  /* create graphics context, so that we may draw in this window */
  gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
  XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
  XSetLineAttributes( display_ptr, gc, 6, LineSolid, CapRound, JoinRound);
  /* and three other graphics contexts, to draw in yellow and red and grey*/
  gc_yellow = XCreateGC( display_ptr, win, valuemask, &gc_yellow_values);
  XSetLineAttributes(display_ptr, gc_yellow, 3, LineSolid,CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "yellow", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color yellow\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_yellow, tmp_color1.pixel );
  gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
  XSetLineAttributes( display_ptr, gc_red, 3, LineSolid, CapRound, JoinRound);
  if( XAllocNamedColor( display_ptr, color_map, "red", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color red\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_red, tmp_color1.pixel );
  gc_grey = XCreateGC( display_ptr, win, valuemask, &gc_grey_values);
  if( XAllocNamedColor( display_ptr, color_map, "light grey", 
			&tmp_color1, &tmp_color2 ) == 0 )
    {printf("failed to get color grey\n"); exit(-1);} 
  else
    XSetForeground( display_ptr, gc_grey, tmp_color1.pixel );

  /* and now it starts: the event loop */
  while(1)
    { XNextEvent( display_ptr, &report );
      switch( report.type )
	{
	case Expose:
          /* re-draw the rectangles. This event happens
             each time some part of the window gets exposed (becomes visible) */

    for( i=0; i< rectanglenumber; i++)
	  {
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][2],
			rectangles[i][0], rectangles[i][3] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][1], rectangles[i][2],
			rectangles[i][1], rectangles[i][3] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][2],
			rectangles[i][1], rectangles[i][2] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][3],
			rectangles[i][1], rectangles[i][3] );
          }
	  if( pathlength > 0 )
	  { for( i=0; i< pathlength -1; i++)
	    {
	      XDrawLine(display_ptr, win, gc_red,
			path[i][0], path[i][1],
			path[i+1][0], path[i+1][1]);
	    }
	  }
	  else{

	    printf("No path defined\n");
          }
          break;
        case ConfigureNotify:
          /* This event happens when the user changes the size of the window*/
          win_width = report.xconfigure.width;
          win_height = report.xconfigure.height;
          break;
        case ButtonPress:
          /* This event happens when the user pushes a mouse button. I draw
            a circle to show the point where it happened, but do not save 
            the position; so when the next redraw event comes, these circles
	    disappear again. */
          {  /*
             int x, y;
  	     x = report.xbutton.x;
             y = report.xbutton.y;
             */
	     printf("calling zoopath function \n");

	     pathlength = zoopath(rectangles, rectanglenumber, path, 1000);
             printf("zoopath generated path of length %d\n", pathlength);
          }
          /* no break here; fall through to redraw case */
        default: /* redraw everything */
	  for( i=0; i< rectanglenumber; i++)
	  {
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][2],
			rectangles[i][0], rectangles[i][3] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][1], rectangles[i][2],
			rectangles[i][1], rectangles[i][3] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][2],
			rectangles[i][1], rectangles[i][2] );
	      XDrawLine(display_ptr, win, gc,
			rectangles[i][0], rectangles[i][3],
			rectangles[i][1], rectangles[i][3] );
          }
	  if( pathlength > 0 )
	  { for( i=0; i< pathlength -1; i++)
	    {

	      XDrawLine(display_ptr, win, gc_red,
			path[i][0], path[i][1],
			path[i+1][0], path[i+1][1]);
	    }
	  }
	  else{

	    printf("No path defined\n");

	  
          break;
    }
	}

    }
  exit(0);
}

int zoopath(int a[][4], int b, int c[][2], int d)
{
  // distance matrix
  int dist_matrix[b][b]={0};
  // pick 2 rectangles from the 11 rectangles and calculate shortest distance
  for(int i=0; i<b; i++){
    for(int j=0; j<b; j++){
      // TODO: this part can archive pick all possible 2 rectangles 
      if(i != j){
      // get the center coordinate of each rectangle
      Vertex src, dest;
      src.vex = {int(a[i][0]+(a[i][1]-a[i][0])/2),int(a[i][2]+(a[i][3]-a[i][2])/2),i};
      dest.vex = {int(a[j][0]+(a[j][1]-a[j][0])/2),int(a[j][2]+(a[j][3]-a[j][2])/2),j};
      vector<Edge> edges;

      // get all rectangle vertex(without src and dest's rectangle)
      vector<Vertex> rec_vertex; // save all vertexes of rectangle
      Vertex tmp;
      for(int i=0; i<b; i++){
        if(i != src.vex.id && i != dest.vex.id){
        tmp.vex.x = a[i][0];
        tmp.vex.y = a[i][2];
        tmp.vex.id = i;
        rec_vertex.push_back(tmp);

        tmp.vex.x = a[i][0];
        tmp.vex.y = a[i][3];
        tmp.vex.id = i;
        rec_vertex.push_back(tmp);

        tmp.vex.x = a[i][1];
        tmp.vex.y = a[i][2];
        tmp.vex.id = i;
        rec_vertex.push_back(tmp);

        tmp.vex.x = a[i][1];
        tmp.vex.y = a[i][3];
        tmp.vex.id = i;
        rec_vertex.push_back(tmp);
      }
    }

      //printf("Size of the rectangle vertexes(should be 36) are: %ld\n", rec_vertex.size());

      
      /* find valid edges */
      find_edges(src, dest, edges, rec_vertex);
      //printf("Size of the valide edges (should be) are: %ld\n", edges.size());

      /* for each vertex(withou src and dest), find the neighbor edges */
      for(int i=0; i<rec_vertex.size(); i++){
        for(int j=0; j<edges.size(); j++){
          if(rec_vertex[i].vex.x == edges[j].start.x && rec_vertex[i].vex.y == edges[j].start.y){
            rec_vertex[i].neighbor.push_back(edges[j]);
          }
        }
      }
      for(int j=0; j<edges.size(); j++){
          if(src.vex.x == edges[j].start.x && src.vex.y == edges[j].start.y){
            src.neighbor.push_back(edges[j]);
          }
        }
      for(int j=0; j<edges.size(); j++){
          if(dest.vex.x == edges[j].start.x && dest.vex.y == edges[j].start.y){
            dest.neighbor.push_back(edges[j]);
          }
        }

      
      dist_matrix[i][j] = Dijkastra(src, dest, rec_vertex);
      //printf("Display the distance[1][1]: %f\n", Distance[1][1]);
      //printf("Display the visited: %d\n", visited[1][1]);
      //printf("Distance is: %f\n", dist);

      // release space
      edges.clear();
      rec_vertex.clear();
    }
  }
}
  
  for(int i=0; i<b; i++){
    for(int j=0; j<b; j++){
      printf("%d\t", dist_matrix[i][j]);
    }
    printf("\n");
  }
  /* call Prim algorithm to obtain the vertexes sequence */
  vector<Edge> sequence;
  Prim(b, &dist_matrix[0][0], sequence);

  //printf("Size of sequence: %d\n", sequence.size());
  //printf("Size of 3.id: %d\n", sequence[0].end.id);


  list<int>* adj;
  adj = new list<int>[b];
  for(int i=0; i<sequence.size(); i++){
    for(int j=0; j<b; j++){
      if(j == sequence[i].start.id){
        adj[j].push_back(sequence[i].end.id);
      }
    }
  }

  // check adj
  // for(int j=0; j<b; j++){
  //   list<int>::iterator i;
  //   for (i = adj[j].begin(); i != adj[j].end(); ++i){
  //     printf("Size of adj: %d\n", *i);
  //   }  
  // }

  /* run dfs */
  bool visited[b];
  for(int i=0; i<b; i++){
    visited[i] = false;
  }
  vector<int> index;
  run_dfs(0, visited, adj, index); //0 3 4 5 1 6 2 9 10 8 7 0
  printf("\n");
  delete [] adj;

  index.push_back(0);

  //check index seqence
  printf("Sequence is:");
  for(int i=0; i<index.size(); i++){
    printf("%d - ", index[i]);
  }
  printf("\n");



  /* generate actual path generate actual path generate actual path */

  //int k=0; // temporary pathlength
  //int cc[100][2]; // temporary path
  vector<Point> path_list;

  for(int k=0; k<index.size()-1; k++){
    vector<Vertex> src;
    vector<Vertex> dest;
    Vertex start;
    Vertex end;
    for(int x=0; x<2; x++){
      for(int y=2; y<4; y++){
        start.vex.x = a[index[k]][x];
        start.vex.y = a[index[k]][y];
        start.vex.id = index[k];
        src.push_back(start);

        end.vex.x = a[index[k+1]][x];
        end.vex.y = a[index[k+1]][y];
        end.vex.id = index[k+1];
        dest.push_back(end);
      }
    }
      

      // get all rectangle vertex(with src and dest's rectangle)
      vector<Vertex> rec_vertex; // save all vertexes of rectangle
      Vertex tmp;
      for(int i=0; i<b; i++){
        tmp.vex.x = a[i][0];
        tmp.vex.y = a[i][2];
        tmp.vex.id = i;
        rec_vertex.push_back(tmp);

        tmp.vex.x = a[i][0];
        tmp.vex.y = a[i][3];
        tmp.vex.id = i;
        rec_vertex.push_back(tmp);

        tmp.vex.x = a[i][1];
        tmp.vex.y = a[i][2];
        tmp.vex.id = i;
        rec_vertex.push_back(tmp);

        tmp.vex.x = a[i][1];
        tmp.vex.y = a[i][3];
        tmp.vex.id = i;
        rec_vertex.push_back(tmp);
      }

      //printf("Size of rec_vertex: %d\n", rec_vertex.size());

      int min = 500;
      int distance;
      //int n;
      Point dr1, dr2;
      vector<Edge> edges;
      vector<int> id;
      int count; // for judge if all 4 x 4 vertexes have non valide lines
    
    for(int x=0; x<src.size(); x++){
      for(int y=0; y<dest.size(); y++){
        /* find valid edges */
        find_nonblock_line(src[x], dest[y], edges, rec_vertex);
        /* calculate valid edge's length */
        for(int j=0; j<edges.size(); j++){
          edges[j].weight = int(sqrt(pow((edges[j].end.y-edges[j].start.y),2) + pow((edges[j].end.x-edges[j].start.x),2)));
        }

        if(!edges.empty()){
          for(int idx=0; idx<edges.size(); idx++){
            // if exist valid edges, select it            
              distance = edges[idx].weight;
              //printf("Size of edges: %d\n", edges.size());
              // XDrawLine(display_ptr, win, gc_red,
              //   edges[idx].start.x, edges[idx].start.y,
              //   edges[idx].end.x, edges[idx].end.y);
              if(min>distance){
                min = distance; 
                dr1 = edges[idx].start;
                dr2 = edges[idx].end;         
                // get the final shortest vertexes
              }
            }

            // cc[k][0] = dr1.x;
            // cc[k][1] = dr1.y;
            // cc[k+1][0] = dr2.x;
            // cc[k+1][1] = dr2.y;
          }
          
          // judge which rectangle block our src and dest
          else{
            count++;
            
            continue;
            //printf("It's empty empty empty empty\n");
          }       
      }
    }

    // for all 16 lines. they are all blocked
    // int ind=0;
    // if(count == 16){ 
    //   find_block_id(src[0], dest[0], rec_vertex, id);
    //   printf("All block id is: between cage %d and cage %d\n", src[0].vex.id, dest[0].vex.id);
    //   for(int i=0; i<id.size(); i++){
    //     if(id[i] != src[0].vex.id && id[i] != dest[0].vex.id){
    //       ind = id[i];
    //       printf("Blocked cages are: %d\n", id[i]);
    //       break; // ready get the nearest block cage id
    //     }
    //   }

    //   index.insert(index.begin()+i+1, ind); // insert the block cage id
    //   i--;
    // } // obtain the shortest path ignore the block situation
    // for all 16 lines. they are all blocked
    int ind=0;
    int dir_min = INT_MAX;
    int dir_distance = 0;
    int idx_x, idx_y;
    if(count == 16){
      for(int i=0; i<src.size(); i++){
        for(int j=0; j<dest.size(); j++){
          dir_distance = int(sqrt(pow((dest[j].vex.y-src[i].vex.y),2) + pow((dest[j].vex.x-src[i].vex.x),2)));
          if(dir_min > dir_distance){
            dir_min = dir_distance;
            idx_x = i;
            idx_y = j;
          }
        }
      } 
      find_block_id(src[idx_x], dest[idx_y], rec_vertex, id);
      printf("All block id is: between cage %d and cage %d\n", src[idx_x].vex.id, dest[idx_y].vex.id);
      for(int i=0; i<id.size(); i++){
        if(id[i] != src[idx_x].vex.id && id[i] != dest[idx_y].vex.id){
          ind = id[i];
          printf("Blocked cages are: %d\n", id[i]);
          break; // ready get the nearest block cage id
        }
      }

      index.insert(index.begin()+k+1, ind); // insert the block cage id
      k--;
    } // obtain the shortest path ignore the block situation

    


    

    path_list.push_back(dr1);
    path_list.push_back(dr2);

    for(int i=0; i<id.size(); i++){
      printf("%d is blocked me\n", id[i]);
    }

    //k += 2;
    edges.clear(); // release for next 2 cages
    src.clear(); // release for next 2 cages
    dest.clear(); // release for next 2 cages
    id.clear(); // release for next 2 cages
    count = 0; // update count for next 2 cages

  }


  int path=0;

  printf("K value is: %d\n", path_list.size());
  for(int i=0;i<path_list.size();i++){

    printf("(%d,%d)-", path_list[i].x,path_list[i].y);
  }

  for(int i=0; i<path_list.size(); i++){
    if(path_list[i].x == path_list[i+1].x && path_list[i].y == path_list[i+1].y){
      c[path][0] = path_list[i].x;
      c[path][1] = path_list[i].y;
      i++;
      path++;
    } // same point
    else if(path_list[i].x != path_list[i+1].x && path_list[i].y != path_list[i+1].y && path_list[i].id == path_list[i+1].id){
      c[path][0] = path_list[i].x;
      c[path][1] = path_list[i].y;

      c[path+1][0] = path_list[i].x;
      c[path+1][1] = path_list[i+1].y;

      path += 2;
    } // diagonal line
    else{
      c[path][0] = path_list[i].x;
      c[path][1] = path_list[i].y;
      path++;
    }
  }

  printf("===========\n");

  for(int i=0; i<path; i++){
    printf("(%d,%d)-", c[i][0],c[i][1]);
  }

  path_list.clear();

  return(path);
}


/* find possible edge for src and dest */ 
/* obtain the valid edges for each source and destination 
   for Dijkastra's algorithm */
void find_edges(Vertex src, Vertex dest, vector<Edge> &edges, vector<Vertex> vertexes){
  /* get all rectangle sides */
  vector<Edge> rec_edges;
  Edge rec_tmp;
  for(int i=0; i<vertexes.size(); i++){
    for(int j=0; j<vertexes.size(); j++){
      /* same rectangle(exclude src and dest) */
      if(vertexes[i].vex.id == vertexes[j].vex.id){
        if(vertexes[i].vex.x == vertexes[j].vex.x && vertexes[i].vex.y == vertexes[j].vex.y || vertexes[i].vex.x != vertexes[j].vex.x && vertexes[i].vex.y != vertexes[j].vex.y){
          // same vertex or diagonal
          continue;
        }
        else{
          rec_tmp.start = vertexes[i].vex;
          rec_tmp.end = vertexes[j].vex;
          rec_tmp.weight = int(sqrt(pow((vertexes[j].vex.y-vertexes[i].vex.y),2) + pow((vertexes[j].vex.x-vertexes[i].vex.x),2)));
          rec_edges.push_back(rec_tmp);
        }
      }
      else
        continue;
    }
  }
  //printf("Size of rectangle sides(should be 72) are: %ld\n", rec_edges.size());

  /* get all edges with different rectangle's vertex */
  vector<Edge> candidate_edges;
  Edge edge_tmp;
  for(int i=0; i<vertexes.size(); i++){
    for(int j=0; j<vertexes.size(); j++){
      /* different rectangle's vertex(exclude src and dest) */
      if(vertexes[i].vex.id != vertexes[j].vex.id){
        edge_tmp.start = vertexes[i].vex;
        edge_tmp.end = vertexes[j].vex;
        candidate_edges.push_back(edge_tmp);
      }    
    }
  }
  //printf("Size of candidate edges(1152) are: %ld\n", candidate_edges.size());

  /* get all edges connection src/dest with rectangle's vertex */
  for(int i=0; i<vertexes.size(); i++){
      edge_tmp.start = src.vex;
      edge_tmp.end = vertexes[i].vex;
      candidate_edges.push_back(edge_tmp);
}
  for(int i=0; i<vertexes.size(); i++){
      edge_tmp.start = vertexes[i].vex;
      edge_tmp.end = src.vex;
      candidate_edges.push_back(edge_tmp);
  }
  for(int i=0; i<vertexes.size(); i++){
      edge_tmp.start = vertexes[i].vex;
      edge_tmp.end = dest.vex;
      candidate_edges.push_back(edge_tmp);
}
  for(int i=0; i<vertexes.size(); i++){
      edge_tmp.start = dest.vex;
      edge_tmp.end = vertexes[i].vex;
      candidate_edges.push_back(edge_tmp);
  }
  /* edges connection src and dest */
  edge_tmp.start = src.vex;
  edge_tmp.end = dest.vex;
  candidate_edges.push_back(edge_tmp);

  edge_tmp.start = dest.vex;
  edge_tmp.end = src.vex;
  candidate_edges.push_back(edge_tmp);

  //printf("Size of candidate edges(final 1298) are: %ld\n", candidate_edges.size());


  /* check the valide edges */
  int count = 0;
  int num = 0;
  for(int i=0; i<candidate_edges.size(); i++){
    for(int j=0; j<rec_edges.size(); j++){
      if(!find_valid_edges(rec_edges[j], candidate_edges[i])){
        count++;
      }
      else // not cross
        continue;
    }
    if(count == 0){
      edges.push_back(candidate_edges[i]);
    }
    else{
      num += 1;
    }
    count = 0; // update count 
  } /* check the valide edges end */

  //printf("The num is: %d\n", num);
  //printf("The valid edges(withou rectangle sides) are: %ld\n", edges.size());

  /* calculate valid edge's length */
  for(int i=0; i<edges.size(); i++){
    edges[i].weight = int(sqrt(pow((edges[i].end.y-edges[i].start.y),2) + pow((edges[i].end.x-edges[i].start.x),2)));
  }


  /* add rectangle sides to vector */
  for(int i=0; i<rec_edges.size(); i++){
    edges.push_back(rec_edges[i]);
  }

  //printf("The valid edges are: %ld\n", edges.size());

}/* find_edges function end */


/* selected non-cross edges */
int find_valid_edges(Edge e1, Edge e2){ // e1 means rectangle sides; e2 means candidate edges

  /* orientation(abp)*orientation(abq) < 0 and
         orientation(pqa)*orientation(pqb) < 0.
         Here orientation(ABC) = Ax*By - Ay*Bx + Bx* Cy - By*Cx + Cx*Ay - Cy* Ax
         Here orientation(PQA) = Px*Qy - Py*Qx + Qx* Ay - Qy*Ax + Ax*Py - Ay* Px
      */
      /* a,b = rec_edges[i]; p,q = edges[j]
      */
  if( ( (e1.start.x * e1.end.y - e1.start.y * e1.end.x + 
         e1.end.x * e2.start.y - e1.end.y * e2.start.x +
         e2.start.x * e1.start.y - e2.start.y * e1.start.x) * 
        (e1.start.x * e1.end.y - e1.start.y * e1.end.x +
         e1.end.x * e2.end.y - e1.end.y * e2.end.x +
         e2.end.x * e1.start.y - e2.end.y * e1.start.x ) < 0) 
    && 
      ( (e2.start.x * e2.end.y - e2.start.y * e2.end.x +
         e2.end.x * e1.start.y - e2.end.y * e1.start.x +
         e1.start.x * e2.start.y - e1.start.y * e2.start.x) * 
        (e2.start.x * e2.end.y - e2.start.y * e2.end.x +
         e2.end.x * e1.end.y - e2.end.y * e1.end.x +
         e1.end.x * e2.start.y - e1.end.y * e2.start.x) < 0)) {
        
        return 0; // 0 means these 2 edges cross
      }
      else
        return 1; // 1 means these 2 edges not cross
}
/* find_valid_edges function end */


int Dijkastra(Vertex src, Vertex dest, vector<Vertex> vertexes){
  int Distance[500][500];
  int visited[500][500];

  /* Dijkaster algorithm */
  // initialize Distance and visited
  for(int i=0; i<500; i++){
    for(int j=0; j<500; j++){
      Distance[i][j] = INT_MAX;
      visited[i][j] = 0;
    }
  }

  // check vertexes
  //printf("X: %ld, Y: %ld\n", vertexes[0].vex.x, vertexes[0].vex.y);
  // deal with source vertex
  Distance[src.vex.x][src.vex.y] = 0;
  visited[src.vex.x][src.vex.y] = 2; // 0 means unvisited, 1 means candidate, 2 means decided
  // seek for candidate vertex
  for(int i=0; i<src.neighbor.size(); i++){
    Distance[src.neighbor[i].end.x][src.neighbor[i].end.y] = src.neighbor[i].weight;
    visited[src.neighbor[i].end.x][src.neighbor[i].end.y] = 1;
  }
  // //print src
  // int i = 0;
  // if(print_flag){
  //   c[i][1] = src.vex.x;
  //   c[i][0] = src.vex.y;
  //   printf("source (%ld,%ld) - ", c[i][1], c[i][0]);
  //   i++;
  // }

  for(int n=0; n<37; n++){
    // find the shortest in the candidate
    int min = INT_MAX;
    int x,y; // record the shortest coordinate
    int index;
    for(int i=0; i<500; i++){
      for(int j=0; j<500; j++){
        if(visited[i][j] == 1){
          if(min>Distance[i][j]){
            min = Distance[i][j];
            x = i;
            y = j;
          }
        }
      }
    }
    visited[x][y] = 2; // decide new vertex
    // confirm the index of new vertex in the vector<Vertex> vertexes
    for(int i=0; i<vertexes.size(); i++){ 
      if(vertexes[i].vex.x == x && vertexes[i].vex.y == y){
        index = i;
      }
    }
    Vertex tmp = vertexes[index];
    // find new vertex's neighbor vertex
    for(int i=0; i<tmp.neighbor.size(); i++){
      if(visited[tmp.neighbor[i].end.x][tmp.neighbor[i].end.y] == 1){
        if(Distance[tmp.neighbor[i].end.x][tmp.neighbor[i].end.y] > min + tmp.neighbor[i].weight){
            Distance[tmp.neighbor[i].end.x][tmp.neighbor[i].end.y] = min + tmp.neighbor[i].weight;
          }
        }
        else if(visited[tmp.neighbor[i].end.x][tmp.neighbor[i].end.y] == 0){ // non visit
          Distance[tmp.neighbor[i].end.x][tmp.neighbor[i].end.y] = min + tmp.neighbor[i].weight;
          visited[tmp.neighbor[i].end.x][tmp.neighbor[i].end.y] = 1;
        }
        else
          continue;
        }


    if(x == dest.vex.x && y == dest.vex.y){
        break;
    }
  }
  return(Distance[dest.vex.x][dest.vex.y]);
}


/* Prims algorithm */
void Prim(int b, int* matrix, vector<Edge> &sequence){
  /* initialization */
  int min = INT_MAX;
  int x,y;
  int visited[b] = {0};
  visited[0] = 1;
  for(int j=1; j<b; j++){
    if(min>*(matrix + 0*b + j)){
      min = *(matrix + 0*b + j);
      x = 0;
      y = j;
    }
  }
  visited[y] = 1; // setup as visited
  // push back to sequence
  Edge edge_tmp;
  edge_tmp.start.id = x;
  edge_tmp.end.id = y;
  edge_tmp.weight = *(matrix + 0*b + y);
  sequence.push_back(edge_tmp);

  printf("Sequence is: %d -> %d : %d\n", x, y, *(matrix + 0*b + y));

  /* loop */
  for(int n=0; n<b-2; n++){
    min = INT_MAX; // refresh min distance
    for(int i=0; i<b; i++){
      if(visited[i] != 0){
        for(int j=0; j<b; j++){
          if(visited[j] == 0 && min > *(matrix + i*b + j)){
            min = *(matrix + i*b + j);
            x = i;
            y = j;
          }
        }
      }
    }
    visited[y] = 1;

    edge_tmp.start.id = x;
    edge_tmp.end.id = y;
    edge_tmp.weight = *(matrix + x*b + y);
    sequence.push_back(edge_tmp);

    printf("Sequence is: %d -> %d : %d\n", x, y, *(matrix + x*b + y));
  }
}

/* Prims algorithm ends */


/* Depth First Search */
/* traversal recursion function */
void run_dfs(int v, bool visited[], list<int>* adj, vector<int> &index)
{
    // Mark the current node as visited and
    // print it
    visited[v] = true;
    cout << v << " ";

    index.push_back(v); // record vertex sequence 
 
    // Recur for all the vertices adjacent
    // to this vertex
    list<int>::iterator i;
    for (i = adj[v].begin(); i != adj[v].end(); ++i)
        if (!visited[*i])
            run_dfs(*i, visited, adj, index);
}
/* traversal recursion function ends */


/* find nonblock line for sequence vertexes line */ 
void find_nonblock_line(Vertex src, Vertex dest, vector<Edge> &edges, vector<Vertex> vertexes){
  /* get all rectangle sides */
  vector<Edge> rec_edges;
  Edge rec_tmp;
  for(int i=0; i<vertexes.size(); i++){
    for(int j=0; j<vertexes.size(); j++){
      /* same rectangle(include src and dest) */
      if(vertexes[i].vex.id == vertexes[j].vex.id){
        if(vertexes[i].vex.x == vertexes[j].vex.x && vertexes[i].vex.y == vertexes[j].vex.y || vertexes[i].vex.x != vertexes[j].vex.x && vertexes[i].vex.y != vertexes[j].vex.y){
          // same vertex or diagonal
          continue;
        }
        else{
          rec_tmp.start = vertexes[i].vex;
          rec_tmp.end = vertexes[j].vex;
          rec_tmp.weight = int(sqrt(pow((vertexes[j].vex.y-vertexes[i].vex.y),2) + pow((vertexes[j].vex.x-vertexes[i].vex.x),2)));
          rec_edges.push_back(rec_tmp);
        }
      }
      else
        continue;
    }
  }
  //printf("Size of rectangle sides(should be 72) are: %ld\n", rec_edges.size());

  Edge candidate_edges;
  /* edges connection src and dest */
  candidate_edges.start = src.vex;
  candidate_edges.end = dest.vex;

  // edge_tmp.start = dest.vex;
  // edge_tmp.end = src.vex;
  // candidate_edges.push_back(edge_tmp);


  /* check the valide edges */
  int count = 0;
  int num = 0;
  for(int j=0; j<rec_edges.size(); j++){
    if(!find_valid_edges(rec_edges[j], candidate_edges)){
      count++;
    }
    else // not cross
      continue;
  }
  if(count == 0){
    edges.push_back(candidate_edges);
  }

  /* check the valide edges end */

}/* find_nonblock_line function end */


void find_block_id(Vertex src, Vertex dest, vector<Vertex> vertexes, vector<int> &id){
  /* get all rectangle sides */
  vector<Edge> rec_edges;
  Edge rec_tmp;
  for(int i=0; i<vertexes.size(); i++){
    for(int j=0; j<vertexes.size(); j++){
      /* same rectangle(include src and dest) */
      if(vertexes[i].vex.id == vertexes[j].vex.id){
        if(vertexes[i].vex.x == vertexes[j].vex.x && vertexes[i].vex.y == vertexes[j].vex.y || vertexes[i].vex.x != vertexes[j].vex.x && vertexes[i].vex.y != vertexes[j].vex.y){
          // same vertex or diagonal
          continue;
        }
        else{
          rec_tmp.start = vertexes[i].vex;
          rec_tmp.end = vertexes[j].vex;
          rec_tmp.weight = int(sqrt(pow((vertexes[j].vex.y-vertexes[i].vex.y),2) + pow((vertexes[j].vex.x-vertexes[i].vex.x),2)));
          rec_edges.push_back(rec_tmp);
        }
      }
      else
        continue;
    }
  }
  //printf("Size of rectangle sides(should be 72) are: %ld\n", rec_edges.size());

  Edge candidate_edges;
  /* edges connection src and dest */
  candidate_edges.start = src.vex;
  candidate_edges.end = dest.vex;


  /* check the block id */
  for(int j=0; j<rec_edges.size(); j++){
    if(!find_valid_edges(rec_edges[j], candidate_edges)){
        id.push_back(rec_edges[j].start.id);  
    }
    else // not cross
      continue;
  }

}/* find_block_id function end */