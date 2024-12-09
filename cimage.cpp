#include "component.h"
#include "cimage.h"
#include "bmplib.h"
#include <deque>
#include <iomanip>
#include <iostream>
#include <cmath>

using namespace std;

CImage::CImage(const char* bmp_filename)
{
    //  Note: readRGBBMP dynamically allocates a 3D array
    //    (i.e. array of pointers to pointers (1 per row/height) where each
    //    point to an array of pointers (1 per col/width) where each
    //    point to an array of 3 unsigned char (uint8_t) pixels [R,G,B values])

    // ================================================
    // Call readRGBBMP to initialize img_, h_, and w_;
    img_ = readRGBBMP(bmp_filename, h_, w_);
    // call the read function with specified parameters and assign it to img_ 


    if(img_ == NULL) {
        throw std::logic_error("Could not read input file");
    }

    // Set the background RGB color using the upper-left pixel
    for(int i=0; i < 3; i++) {
        bgColor_[i] = img_[0][0][i];
    }

    // RGB "distance" threshold to continue a BFS from neighboring pixels
    bfsBgrdThresh_ = 60;

    // ================================================
    for (int i =0; i < h_; i++){
      // loop through the height 
      labels_.push_back(vector<int>(w_,-1));
      // use push back to add in vector<int> of width with the value -1
      // this is for the 2D array of pixel labels 
    }



    // ================================================
    count = 0; 
    // initialize to 0 since it is constructor 



}

CImage::~CImage()
{
    // Add code here if necessary
    deallocateImage(img_);
    // call this to destruct 


}

bool CImage::isCloseToBground(uint8_t p1[3], double within) {
    // Computes "RGB" (3D Cartesian distance)
    double dist = sqrt( pow(p1[0]-bgColor_[0],2) +
                        pow(p1[1]-bgColor_[1],2) +
                        pow(p1[2]-bgColor_[2],2) );
    return dist <= within;
}

size_t CImage::findComponents()
{
  // start by looping through everything: 
  for (int i = 0; i < h_; i++){
    for (int j = 0; j < w_; j++){
      while (labels_[i][j] == -1){
        // see if the label array is -1 (default)
        if (!(isCloseToBground(img_[i][j],bfsBgrdThresh_))){
          // if it is, it also has to pass this check 
          // it cannot be the background 
          // if passes both these if statements, create a new component 
          Component newcomp = bfsComponent(i,j,count); 
          components_.push_back(newcomp);
          // add component into array 
          count++; 
          // increase the count of components 
        }
        break; 
      }
    }
  }
  int ret = count+1; 
  return ret; 
  // return the number of components 
}

void CImage::printComponents() const
{
    cout << "Height and width of image: " << h_ << "," << w_ << endl;
    cout << setw(4) << "Ord" << setw(4) << "Lbl" << setw(6) << "ULRow" << setw(6) << "ULCol" << setw(4) << "Ht." << setw(4) << "Wi." << endl;
    for(size_t i = 0; i < components_.size(); i++) {
        const Component& c = components_[i];
        cout << setw(4) << i << setw(4) << c.label << setw(6) << c.ulNew.row << setw(6) << c.ulNew.col
             << setw(4) << c.height << setw(4) << c.width << endl;
    }

}


int CImage::getComponentIndex(int mylabel) const
{
  bool exists; 
  // track to see if component exists 
  for (size_t i = 0; i < components_.size(); i++){
    // loop through components 
    if (components_[i].label == mylabel){
      // compare to see if same 
      exists = true; 
      return i; 
      // make the bool true and return the index 
    }
  }
  if (!exists){
    return -1; 
    // return -1 if the component does not exist 
  }
  /**
  * @brief Get the index of the component with mylabel or     *        -1 if no component with mylabel exists
  * 
  * @param mylabel Label of the component 
  * @return int index in the Component storage
  */
}


//   Add checks to ensure the new location still keeps
//   the entire component in the legal image boundaries
void CImage::translate(int mylabel, int nr, int nc)
{
    // Get the index of specified component
    int cid = getComponentIndex(mylabel);
    if(cid < 0) {
        return;
    }
    int h = components_[cid].height;
    int w = components_[cid].width;

    // ==========================================================
    // CODE TO CHECK IF THE COMPONENT WILL STILL BE IN BOUNDS
    // IF NOT:  JUST RETURN.
    bool in_bounds = false; 
    // variable to track 
    if ((nr >= 0) && (nr + h <= h_)){
      // check r bounds first 
      if ((nc >= 0) && (nc + w <= w_)){
        // then check c bounds 
        in_bounds = true; 
      }
    }
    if (!in_bounds){
      return; 
      // if statement for if not in bounds, just return 
    }

    // ==========================================================

    // If we reach here we assume the component will still be in bounds
    // so we update its location.
    Location nl(nr, nc);
    components_[cid].ulNew = nl;
}

void CImage::forward(int mylabel, int delta)
{
    int cid = getComponentIndex(mylabel);
    if(cid < 0 || delta <= 0) {
        return;
    }
    int accdelt = min(delta, count-cid-1);
    // find the delta to make sure it does not go out of bounds 
    for (int i = 0; i < accdelt; i++){
      swap(components_[cid+i],components_[cid+1+i]);
      // loop through elements and swap to go forward 
    }
}

void CImage::backward(int mylabel, int delta)
{
    int cid = getComponentIndex(mylabel);
    if(cid < 0 || delta <= 0) {
        return;
    }
    int accdelt = min(delta, cid);
    // again update delta to make sure in bounds 
    for (int i =0; i < accdelt; i++){
      swap(components_[cid - i], components_[cid - i -1]);
      // same thing as the foward function but now it is backwards so subtract
    }
}

void CImage::save(const char* filename)
{
    // Create another image filled in with the background color
    uint8_t*** out = newImage(bgColor_);

    for (size_t k = 0; k < components_.size(); k++){
      Component newcomp = components_[k]; 
      // loop through and create components 
      bool valid = true; 
      // create a variable to see if the bounds remain valid 
      // if statements for bounds checking: 
      if ((newcomp.ulOrig.row < 0) || (newcomp.ulOrig.col < 0) || ((newcomp.ulOrig.row + newcomp.height) > h_) || ((newcomp.ulOrig.col + newcomp.width) > w_) ||
      (newcomp.ulNew.row < 0) || (newcomp.ulNew.col < 0) || ((newcomp.ulNew.row + newcomp.height) > h_) || (newcomp.ulNew.col + newcomp.width) > w_){
        valid = false; 
        // out of bounds if any of them are true 
      }
      if (!valid){
        continue; 
        // continue if bounds are not valid 
      }
      for (int i = 0; i < newcomp.height; i++){
        for (int j = 0; j < newcomp.width; j++){
          // loop through everything 
          // will now find the new and old positions 
          int oldr = newcomp.ulOrig.row + i; 
          int oldc = newcomp.ulOrig.col + j; 
          int newr = newcomp.ulNew.row + i; 
          int newc = newcomp.ulNew.col + j; 

          // now check to make sure positions are still valid 
          if ((oldr >= 0) && (oldr < h_) 
          && (oldc >= 0) && (oldc < w_)
          && (newr >= 0) && (newr < h_)
          && (newc >= 0) && (newc < w_)
          && (labels_[oldr][oldc] == newcomp.label)){
            // can now assign the values 
            out[newr][newc][0] = img_[oldr][oldc][0];
            out[newr][newc][1] = img_[oldr][oldc][1];
            out[newr][newc][2] = img_[oldr][oldc][2]; 
          }
        }
      }
    }

    writeRGBBMP(filename, out, h_, w_);
    deallocateImage(out); 
    // make sure to deallocate the memory 
    
}

// Creates a blank image with the background color
uint8_t*** CImage::newImage(uint8_t bground[3]) const
{
    uint8_t*** img = new uint8_t**[h_];
    for(int r=0; r < h_; r++) {
        img[r] = new uint8_t*[w_];
        for(int c = 0; c < w_; c++) {
            img[r][c] = new uint8_t[3];
            img[r][c][0] = bground[0];
            img[r][c][1] = bground[1];
            img[r][c][2] = bground[2];
        }
    }
    return img;
}

void CImage::deallocateImage(uint8_t*** img) const
{
    for (int i = 0; i < h_; i++){
      for (int j = 0; j < w_; j++){
        // loop through everything 
        delete[] img[i][j];
        // delete col 
      }
      delete[] img[i];
      // delete row 
    }
    delete[] img; 
    // delete 
}

Component CImage::bfsComponent(int pr, int pc, int mylabel)
{
    // Arrays to help produce neighbors easily in a loop
    // by encoding the **change** to the current location.
    // Goes in order N, NW, W, SW, S, SE, E, NE
    int neighbor_row[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int neighbor_col[8] = {0, -1, -1, -1, 0, 1, 1, 1};
    Location loc(pr,pc);
    labels_[pr][pc] = mylabel; 
    deque<Location> myqueue = {loc};
    int rmax = pr; 
    int rmin = pr; 
    int cmax = pc; 
    int cmin = pc; 
    // initialize all necessary variables 

    while(!myqueue.empty()){
      // keep going until nothing in queue
      Location curr = myqueue.front(); 
      myqueue.pop_front(); 
      // get locatoin and add to frnnt of queue 

      // update bounds 
      rmax = max(rmax, curr.row);
      rmin = min(rmin, curr.row); 
      cmax = max(cmax, curr.col);
      cmin = min(cmin, curr.col);

      // check the neighboring now 
      for (int i = 0; i < 8; i++){
        int nr = curr.row + neighbor_row[i];
        int nc = curr.col + neighbor_col[i]; 
        if ((nr >= 0) && (nr < h_) && (nc >= 0) && (nc < w_) && (labels_[nr][nc] == -1) && (!(isCloseToBground(img_[nr][nc], bfsBgrdThresh_)))){
          labels_[nr][nc] = mylabel; 
          myqueue.push_back(Location(nr,nc));
        }
      }
    }

    Location ul(rmin, cmin);
    int reth = rmax - rmin + 1; 
    int retw = cmax - cmin + 1; 
    return Component(ul, reth, retw, mylabel); 
}

// Debugging function to save a new image
void CImage::labelToRGB(const char* filename)
{
    //multiple ways to do this -- this is one way
    vector<uint8_t[3]> colors(components_.size());
    for(unsigned int i=0; i<components_.size(); i++) {
        colors[i][0] = rand() % 256;
        colors[i][1] = rand() % 256;
        colors[i][2] = rand() % 256;
    }

    for(int i=0; i<h_; i++) {
        for(int j=0; j<w_; j++) {
            int mylabel = labels_[i][j];
            if(mylabel >= 0) {
                img_[i][j][0] =  colors[mylabel][0];
                img_[i][j][1] =  colors[mylabel][1];
                img_[i][j][2] =  colors[mylabel][2];
            } else {
                img_[i][j][0] = 0;
                img_[i][j][1] = 0;
                img_[i][j][2] = 0;
            }
        }
    }
    writeRGBBMP(filename, img_, h_, w_);
}

const Component& CImage::getComponent(size_t i) const
{
    if(i >= components_.size()) {
        throw std::out_of_range("Index to getComponent is out of range");
    }
    return components_[i];
}

size_t CImage::numComponents() const
{
    return components_.size();
}

void CImage::drawBoundingBoxesAndSave(const char* filename)
{
    for(size_t i=0; i < components_.size(); i++){
        Location ul = components_[i].ulOrig;
        int h = components_[i].height;
        int w = components_[i].width;
        for(int i = ul.row; i < ul.row + h; i++){
            for(int k = 0; k < 3; k++){
                img_[i][ul.col][k] = 255-bgColor_[k];
                img_[i][ul.col+w-1][k] = 255-bgColor_[k];

            }
            // cout << "bb " << i << " " << ul.col << " and " << i << " " << ul.col+w-1 << endl; 
        }
        for(int j = ul.col; j < ul.col + w ; j++){
            for(int k = 0; k < 3; k++){
                img_[ul.row][j][k] = 255-bgColor_[k];
                img_[ul.row+h-1][j][k] = 255-bgColor_[k];

            }
            // cout << "bb2 " << ul.row << " " << j << " and " << ul.row+h-1 << " " << j << endl; 
        }
    }
    writeRGBBMP(filename, img_, h_, w_);
}
