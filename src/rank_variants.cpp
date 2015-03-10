#include <Rcpp.h>
using namespace Rcpp;


// [[Rcpp::export]]
Rcpp::DataFrame rank_variants(Rcpp::DataFrame variants,
                              Rcpp::NumericVector ends,
                              Rcpp::NumericVector score){
  // Rank variants by window and by a vector of scores.
  // Input is a DataFrame of variant information which includes
  // the chromosomal position of each variant.
  // This may be originally derived from a vcf file.  
  // It should also contain information on whether each variant has been masked.
  // Also input is a vector of window ending positions and a vector of scores for each
  // variant.  The scores may be extracted from a vcf file, such
  // as the genotype quality filed (GQ), or any sort of composite score.
  // The variants with the greatest score are given the lowest
  // rank order.
  
  Rcpp::NumericVector pos = variants["POS"];
  Rcpp::LogicalVector mask = variants["mask"];
  
  Rcpp::NumericVector win_num(score.size());  // Vector of window numbers.
  Rcpp::NumericVector win_rank(score.size()); // Vector to be returned as per window rank.
  Rcpp::NumericVector rank;   // Vector to hold information for a single 
                              // window which will be sorted.
  int win = 0;
  int i = 0;
  int j = 0;

  // Iterate to first window.
  while(pos(0) > ends(win)){win++;}
  win_num(0) = win;

  for(i=1; i<score.size(); i++){
    Rcpp::checkUserInterrupt();
    if( pos(i) < ends(win) ){
        win_num(i) = win;
//      rank.push_back(score(i));
//      rank[pos(i)] = score(i);
    } else {
//      std::sort(rank.begin(), rank.end());
//      for(j=0; j<rank.size(); j++){
//        Rcout << rank(j) << " ";
//      }
//      Rcout << "\n";
      
      while(pos(i) > ends(win)){win++;}
      win_num(i) = win;
//      win++;
//      win = win + 1;

    }
//    Rcout << "variant: " << i << ", position: " << pos(i) << ", window number: " << win_num(i) << "\n";
  }
  
  for(i=0; i<score.size(); i++){
    Rcout << "variant: " << i << ", position: " << pos(i) << ", window number: " << win_num(i) << "\n";
  }
  Rcout << "win_num is: " << win_num.size()  << " elements long\n";
  
//  return DataFrame::create(variants, _["window_number"]=win_num, _["window_rank"]=win_rank);
  return Rcpp::DataFrame::create(variants, _["window_number"]=win_num);
//  return variants;
}




