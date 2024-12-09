#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <cstdlib>
#include "cimage.h"
#include "bmplib.h"

using namespace std;
void doInputLoop(std::istream& istr, CImage& img); // prototype

int main(int argc, char *argv[])
{
    if(argc < 2) {
        cout << "Usage ./compedit in_img_filename <option option-args>" << endl;
        return 1;
    }

    // ===================
    CImage img1(argv[1]);
    // create and image and the argument is the file name: argv[1]


    // ===================
    // Call findComponents
    img1.findComponents();
    // simple call with the image (function takes no parameters)


    // ===================
    //   Option handling
    int option = 0;
    if(argc >= 3) {
        option = atoi(argv[2]);
    }
    if(option == 1) {
        img1.printComponents();
        return 0;
    }
    else if(option == 2 || option == 3) {
        if(argc < 4) {
            cout << "Please provide an extra argument of the filename to save the image." << endl;
        }
        else if(option == 2){
            img1.labelToRGB(argv[3]);
        }
        else if(option == 3){
            img1.drawBoundingBoxesAndSave(argv[3]);
        }
        return 0;
    }

		// ===================
		// Setup input stream
		if(option == 4) {
        if(argc < 4) {
            cout << "Please provide an extra argument of the command filename." << endl;
            return 0;
        }        
        string cmd_error = "Bad command filename";

        // ===================
        // Create appropriate input stream. Print the string cmd_error and return 0
        // if the file can't be opened

        ifstream inputfile(argv[3]);
        // get the cmd_file 
        if(!(inputfile.is_open())){
          // see if the file can or cannot be opened 
            cout << cmd_error << endl;
            return 0;
        }
        // ===================
        //  Pass the input stream you created
        doInputLoop(inputfile, img1);  
        // ===================
        //  Close the file
        inputfile.close();

		}
    else {
        doInputLoop(cin, img1);
    }

    return 0;
}

void doInputLoop(std::istream& istr, CImage& img)
{
    // ===================
    // Implementation to process user commands
    //  of t - translate, f - forward, b - backward, s - save, and
    //  any other input letter to quit.
    string option_error = "Bad option";
    char option = 'q';
    int cid;
    bool again = true;
    do {
        img.printComponents();
        cout << "\nEnter a command [t,f,b,s,q]: " << endl;
        istr >> option;
        if(option == 't') {
            int nr, nc;
            // ==================
            // Read cid, nr, and nc from the input stream,
            //       printing "Bad option" and breaking from the 
            //       loop if any of the input is not read successfully
            istr >> cid >> nr >> nc; 
            // read in the values 
            if (istr.fail()){
              // check if the input cannot be read 
              cout << option_error << endl; 
              // give out error message if so 
              break; 
            }
            img.translate(cid, nr, nc);
        }
        else if(option == 'f' || option == 'b') {
            int delta;
            // ==================
            // Read cid and delta from the input stream
            //       printing "Bad option" and breaking from the 
            //       loop if any of the input is not read successfully
            istr >> cid >> delta; 
            // read in the values 
            if (istr.fail()){
              cout << option_error << endl; 
              break; 
              // check to see if input can be read, give error message if it cannot 
            }



            if(option == 'f') {
                img.forward(cid, delta);
            }
            else {
                img.backward(cid, delta);
            }
        }
        else if(option == 's') {
            string filename;
            // ==================
            // Read filename from the input stream,
            //       printing "Bad option" and breaking from the 
            //       loop if any of the input is not read successfully
            istr >> filename; 
            // read in values 
            if (istr.fail()){
              cout << option_error << endl;
              break; 
              // check if input can be read, put out error message if fails 
            }


            img.save(filename.c_str());
        }
        else {
            again = false;
        }
    } while(again);
}
