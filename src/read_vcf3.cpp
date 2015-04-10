#include <Rcpp.h>
#include <fstream>
#include <zlib.h>
#include "common.h"
// #include <iostream>

using namespace Rcpp;

// Number of records to report progress at.
const int nreport = 1000;

/* Size of the block of memory to use for reading. */
#define LENGTH 0x1000


// [[Rcpp::export]]
Rcpp::NumericVector vcf_stats_gz(std::string x) {
  
  Rcpp::NumericVector stats(4);
  stats.names() = Rcpp::StringVector::create("meta", "header", "variants", "columns");
  
  
    gzFile file;
    file = gzopen (x.c_str(), "r");
    if (! file) {
//        fprintf (stderr, "gzopen of '%s' failed: %s.\n", x, strerror (errno));
        Rcerr << "gzopen of " << x << "failed: " << strerror (errno) << ".\n";
//            exit (EXIT_FAILURE);
    }
    while (1) {
        int err;                    
        int bytes_read;
        unsigned char buffer[LENGTH];
        bytes_read = gzread (file, buffer, LENGTH - 1);
        buffer[bytes_read] = '\0';
//        printf ("%s", buffer);
        Rcout << buffer;
        Rcout << "\n\n";
        if (bytes_read < LENGTH - 1) {
            if (gzeof (file)) {
                break;
            }
            else {
                const char * error_string;
                error_string = gzerror (file, & err);
                if (err) {
//                    fprintf (stderr, "Error: %s.\n", error_string);
                    Rcerr << "Error: " << error_string << ".\n";
//                    exit (EXIT_FAILURE);
                }
            }
        }
    }
    gzclose (file);
//    return 0;
  std::string tmp = "phrase with a tab\tto split on";
//  std::vector < std::string > svec = common::strsplit(tmp);
//  std::vector < std::string > svec;
//  svec = 
//  std::string svec;
//  common::strsplit(tmp.c_str());
//  common::strsplit(tmp);
//  common::foo();
  std::vector<int> a;
  fun(a);
  Rcout << "Vector out: " << a[0] << ", " << a[1] << "\n";

//  char myword[] = { 'H', 'e', 'l', 'l', 'o', '\0' };

  std::string mystring;
  std::vector < std::string > svec;
  char split = '\n'; // Must be single quotes!

//  common::foo2( myword, svec, mystring );
  common::strsplit(mystring, svec, split);
  
  Rcout << svec[0] << "\n";
  
  return stats;
}


// [[Rcpp::export]]
Rcpp::NumericVector vcf_stats(std::string x) {
  // Scroll through file and collect the number of
  // meta lines, the line number for the header,
  // the number of variant rows and the number of 
  // columns in the tabular region.
  
  Rcpp::NumericVector stats(4);
  stats.names() = Rcpp::StringVector::create("meta", "header", "variants", "columns");

  std::string line;  // String for reading file into
  long int i = 0;
  int j = 0;

  std::ifstream myfile;
  myfile.open (x.c_str(), std::ios::in);

  if (!myfile.is_open()){
    Rcout << "Unable to open file";
  }
  
  // Loop over the file.
  while ( getline (myfile,line) ){
    Rcpp::checkUserInterrupt();
    if(line[0] == '#' && line[1] == '#'){
      stats[0]++;
    } else if (line[0] == '#'){
      stats[1] = stats[0] + 1;
      // Count the columns in the header.
      for(j = 0; j < line.size(); j++ ){
        if(line[j] == '\t'){
          stats[3]++;
        }
      }
      stats[3]++;
    } else {
      stats[2]++;
    }
    i++;
    
    if( i % nreport == 0){
      Rcout << "\rProcessed line: " << i;
    }
  }
  myfile.close();

  Rcout << "\rProcessed line: " << i;
  Rcout << "\nAll lines processed.\n";


//  Rcout << "Line: " << line << "\n";
  
  return stats;
}



// [[Rcpp::export]]
Rcpp::StringVector vcf_meta(std::string x, Rcpp::NumericVector stats) {
  // Read in the meta lines.
  // stats consists of elements ("meta", "header", "variants", "columns");
  
  Rcpp::StringVector meta(stats[0]);
  std::string line;  // String for reading file into
  
  std::ifstream myfile;
  myfile.open (x.c_str(), std::ios::in);

  if (!myfile.is_open()){
    Rcout << "Unable to open file";
  }
  
  // Loop over the file.
  int i = 0;
  while ( i < stats[0] ){
    Rcpp::checkUserInterrupt();
//    Rcout << i << "\n";
    getline (myfile,line);
    meta(i) = line;
    i++;
    
    if( i % nreport == 0){
      Rcout << "\rProcessed meta line: " << i;
    }

  }
  myfile.close();
  Rcout << "\rProcessed meta line: " << i;
  Rcout << "\nMeta lines processed.\n";

  return meta;
}



std::vector < std::string > tabsplit(std::string line, int elements){
  // Split a string into tabs.
  
  std::vector < std::string > stringv(elements);
//  Rcout << "Genotype: " << line << "\n";

  int start=0;
  int j = 0;
//  char c;
  for(int i=1; i<line.size(); i++){
//    c = line[i];
    if( line[i] == '\t'){
      std::string temp = line.substr(start, i - start);
//      Rcout << "  i: " << i << ", temp: " << temp << "\n";
      stringv[j] = temp;
      j++;
      start = i+1;
      i = i+1;
    }
  }

  // Handle last element.
  std::string temp = line.substr(start, line.size());
//  Rcout << "  temp: " << temp << "\n";
//  stringv.push_back(temp);
  stringv[j] = temp;
  
//  for(j=0; j<stringv.size(); j++){ Rcout << "j: " << j << " " << stringv[j] << "\n"; }
  
  return stringv;
}





// [[Rcpp::export]]
Rcpp::DataFrame vcf_body(std::string x, Rcpp::NumericVector stats) {
  // Read in the fixed and genotype portion of the file.

  // Stats contains:
  // "meta", "header", "variants", "columns"

  Rcpp::CharacterVector   chrom(stats[2]);
  Rcpp::IntegerVector  pos(stats[2]);
  Rcpp::StringVector   id(stats[2]);
  Rcpp::StringVector   ref(stats[2]);
  Rcpp::StringVector   alt(stats[2]);
  Rcpp::NumericVector  qual(stats[2]);
  Rcpp::StringVector   filter(stats[2]);
  Rcpp::StringVector   info(stats[2]);

//  if(stats[3])
  Rcpp::CharacterMatrix gt(stats[2], stats[3] - 8);
  
  std::string line;  // String for reading file into

  // Open file.
  std::ifstream myfile;
  myfile.open (x.c_str(), std::ios::in);

  if (!myfile.is_open()){
    Rcout << "Unable to open file";
  }

  // Iterate past meta.
  int i = 0;
  int j = 0;
  while ( i < stats[0] ){
    getline (myfile,line);
    i++;
  }
  
  // Get header.
  getline (myfile,line);
  std::string header = line;
  
  // Get body.
  i = 0;
  char buffer [50];

  while ( getline (myfile,line) ){
    Rcpp::checkUserInterrupt();
    std::vector < std::string > temps = tabsplit(line, stats[3]);

    if(temps[0] == "."){
      chrom[i] = NA_STRING;
    } else {
      chrom[i] = temps[0];
    }
    if(temps[1] == "."){
      pos[i] = NA_INTEGER;
    } else {
        pos[i] = atoi(temps[1].c_str());
    }
    if(temps[2] == "."){
      id[i] = NA_STRING;
    } else {
      id[i] = temps[2];
    }
    if(temps[3] == "."){
      ref[i] = NA_STRING;
    } else {
      ref[i] = temps[3];
    }
    if(temps[4] == "."){
      alt[i] = NA_STRING;
    } else {
      alt[i] = temps[4];
    }
    if(temps[5] == "."){
      qual[i] = NA_REAL;
    } else {
      qual[i] = atof(temps[5].c_str());
    }
    if(temps[6] == "."){
      filter[i] = NA_STRING;
    } else {
      filter[i] = temps[6];
    }
    if(temps[7] == "."){
      info[i] = NA_STRING;
    } else {
      info[i] = temps[7];
    }

    for(j=8; j<stats[3]; j++){
      gt(i, j-8) = temps[j];
//      body(i, j-8) = temps[j];
    }
    i++;
    
    if( i % nreport == 0){
      Rcout << "\rProcessed variant: " << i;
    }

  }
  myfile.close();
  
  Rcout << "\rProcessed variant: " << i;
  Rcout << "\nAll variants processed\n";
  
  Rcpp::DataFrame df1 = Rcpp::DataFrame::create(
    _["CHROM"]= chrom,
    _["POS"]= pos,
    _["ID"] = id,
    _["REF"] = ref,
    _["ALT"] = alt,
    _["QUAL"] = qual,
    _["FILTER"] = filter,
    _["INFO"] = info,
    gt);

  std::vector < std::string > temps = tabsplit(header, stats[3]);
  temps[0].erase(0,1);
  df1.names() = temps;
  
  Rcout << "Rcpp::DataFrame created.\n";
  
  return df1;
}




// [[Rcpp::export]]
int read_to_line(std::string x) {
  std::string line;  // String for reading file into
  long int i = 0;

  std::ifstream myfile;
  myfile.open (x.c_str(), std::ios::in);

  if (!myfile.is_open()){
    Rcout << "Unable to open file";
  }
  
  // Loop over the file.
  while ( getline (myfile,line) ){
    Rcpp::checkUserInterrupt();
//    Rcout << line << "\n";
    i++;
  }

  myfile.close();
  return i;
}






// [[Rcpp::export]]
Rcpp::CharacterMatrix ram_test(int nrow=1, int ncol=1) {
  Rcpp::CharacterMatrix gt(nrow, ncol);
  
  return(gt);
}


Rcpp::StringMatrix DataFrame_to_StringMatrix( Rcpp::DataFrame df ){
  Rcpp::StringVector sv = df(0);
  Rcpp::StringMatrix sm(sv.size(), df.size());
  
  sm.attr("col.names") = df.attr("col.names");
  sm.attr("row.names") = df.attr("row.names");

  for(int i=0; i < df.size(); i++){
    sv = df(i);
    for(int j=0; j < sv.size(); j++){
      sm(j, i) = sv(j);
    }
  }

  return sm;
}


// [[Rcpp::export]]
void write_vcf_body( Rcpp::DataFrame fix, Rcpp::DataFrame gt, std::string filename , int mask=0 ) {
//int write_vcf_body( Rcpp::DataFrame fix, Rcpp::DataFrame gt, std::string filename , int mask=0 ) {

  // fix DataFrame
  Rcpp::StringVector chrom  = fix["CHROM"];
  Rcpp::StringVector pos    = fix["POS"];
  Rcpp::StringVector id     = fix["ID"];
  Rcpp::StringVector ref    = fix["REF"];
  Rcpp::StringVector alt    = fix["ALT"];
  Rcpp::StringVector qual   = fix["QUAL"];
  Rcpp::StringVector filter = fix["FILTER"];
  Rcpp::StringVector info   = fix["INFO"];

  // gt DataFrame
  Rcpp::StringMatrix gt_cm = DataFrame_to_StringMatrix(gt);
  Rcpp::StringVector column_names(gt.size());
  column_names = gt.attr("names");
//  column_names = gt_cm.attr("col.names");
//  delete gt;
  
  int i = 0;
  int j = 0;

  // Uncompressed.
  std::ofstream myfile;
  myfile.open (filename.c_str(), std::ios::out | std::ios::app | std::ios::binary);
  
//  gzFile *fi = (gzFile *)gzopen("file.gz","wb");
  

  for(i=0; i<chrom.size(); i++){
    Rcpp::checkUserInterrupt();
    if(mask == 1 && filter(i) == "PASS" ){
      // Don't print variant.
    } else {
      myfile << chrom(i);
      myfile << "\t";
      myfile << pos(i);
      myfile << "\t";
      if(id(i) == NA_STRING){
        myfile << ".";
        myfile << "\t";
      } else {
        myfile << id(i);
        myfile << "\t";
      }
      myfile << ref(i);
      myfile << "\t";
      myfile << alt(i);
      myfile << "\t";
      if(qual(i) == NA_STRING){
        myfile << ".";
        myfile << "\t";
      } else {
        myfile << qual(i);
        myfile << "\t";
      }
      if(filter(i) == NA_STRING){
        myfile << ".";
        myfile << "\t";
      } else {
        myfile << filter(i);
        myfile << "\t";
      }
      if(info(i) == NA_STRING){
        myfile << ".";
        myfile << "\t";
      } else {
        myfile << info(i);
      }
      
      // gt region.
      myfile << "\t";
      myfile << gt_cm(i, 0);
      for(j=1; j<column_names.size(); j++){
        myfile << "\t";
        myfile << gt_cm(i, j);
      }

      myfile << "\n";
    }
  }

  myfile.close();
  
  return;
}



// [[Rcpp::export]]
void write_vcf_body_gz( Rcpp::DataFrame fix, Rcpp::DataFrame gt, std::string filename , int mask=0 ) {
  // http://stackoverflow.com/a/5649224
  
  // fix DataFrame
  Rcpp::StringVector chrom  = fix["CHROM"];
  Rcpp::StringVector pos    = fix["POS"];
  Rcpp::StringVector id     = fix["ID"];
  Rcpp::StringVector ref    = fix["REF"];
  Rcpp::StringVector alt    = fix["ALT"];
  Rcpp::StringVector qual   = fix["QUAL"];
  Rcpp::StringVector filter = fix["FILTER"];
  Rcpp::StringVector info   = fix["INFO"];
  
  // gt DataFrame
  Rcpp::StringMatrix gt_cm = DataFrame_to_StringMatrix(gt);
  Rcpp::StringVector column_names(gt.size());
  column_names = gt.attr("names");
  
  int i = 0;
  int j = 0;
  
  gzFile *fi = (gzFile *)gzopen(filename.c_str(),"ab");
//  gzFile *fi = (gzFile *)gzopen(filename.c_str(),"abw");
  for(i=0; i<chrom.size(); i++){
    Rcpp::checkUserInterrupt();
    if(mask == 1 && filter(i) != "PASS" ){
      // Don't print variant.
    } else {
      std::string tmpstring;
      tmpstring = chrom(i);
      tmpstring = tmpstring + "\t" + pos(i) + "\t";
      if(id(i) == NA_STRING){
        tmpstring = tmpstring + ".";
      } else {
        tmpstring = tmpstring + id(i);
      }
      tmpstring = tmpstring + "\t" + ref(i) + "\t" + alt(i) + "\t";
      if(qual(i) == NA_STRING){
        tmpstring = tmpstring + "." + "\t";
      } else {
        tmpstring = tmpstring + qual(i) + "\t";
      }
      if(filter(i) == NA_STRING){
        tmpstring = tmpstring + "." + "\t";
      } else {
        tmpstring = tmpstring + filter(i) + "\t";
      }
      tmpstring = tmpstring + info(i);

      // gt portion
      for(j=0; j<column_names.size(); j++){
        tmpstring = tmpstring + "\t" + gt_cm(i, j);
      }


//      gzwrite(fi,"my decompressed data",strlen("my decompressed data"));
//      gzwrite(fi,"\n",strlen("\n"));
//      std::string tmpstring = "test string\n";
      gzwrite(fi, (char *)tmpstring.c_str(), tmpstring.size());
      
      gzwrite(fi,"\n",strlen("\n"));
    }
  }
  gzclose(fi);
  
  
  return;
}
