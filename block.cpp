#include <iostream>
#include <bits/stdc++.h>
#include <vector>
#include <algorithm>
#include <set>
#include <map>

using namespace std;

class Transaction
{
    public : 
        string tx_id ; 
        int fee ; 
        int weight ; 
        vector<string> parents ;
} ; 

string fileName = "mempool.csv";
double max_wt = 4000000.0;

// input is txn from mempool as a vector 
// 1. creating a Transaction object for the txn vector 
// 2. Creating a pair ( tx_id -- pointer to txn object ) 

pair <string , Transaction *> add_to_umap( vector<string> &row ) 
{
    auto txn = new Transaction() ; 
    txn->tx_id = row[0] ; 
    txn->fee = stoi(row[1]) ;
    txn->weight = stoi(row[2]) ; 
    vector<string> p ;

    for( int i = 3 ; i < row.size() ; i++ )
        p.push_back( row[i] ) ; 
    
    txn->parents = p ; 
    return{row[0] , txn}; 

}

// Reading the MEMPOOL and pasting it into unordered map containing pair values

void Create_row_vector_and_add_to_umap( string fileName ,unordered_map< string , Transaction * > &umap ) 
{
    ifstream fin(fileName) ; 
    string line, word ; 
    vector<string> row ; 
    getline( fin , line ) ; 
    
    while( getline(fin,line) )
    {
        row.clear() ; // haven't cleared the row - stock of rows might have interrupted the program
        stringstream s(line) ; 
        while( getline( s , word , ',') )
        {
            row.push_back( word ) ;
            word.clear() ; 
        }
        pair< string , Transaction * > p = add_to_umap( row ) ; 
        umap[p.first] = p.second ; 
        line.clear() ; 
    }
    fin.close() ; 
    cout << "total no.of transaction read : " << umap.size() << endl ; 
}

bool allow_txn( Transaction * txn , set<string> &block_txns_set ) 
{
    for( auto parent : txn->parents ) 
    {
        if( block_txns_set.find(parent) == block_txns_set.end() )
            return( false ) ;
    }
    return( true ) ;
}

void block_write( string fileName , vector<string> &block_txns_vector ) 
{
    ofstream fout(fileName) ;
    for( auto tx_id : block_txns_vector ) 
        fout << tx_id << endl ; 
    fout.close() ; 
}

int main() 
{
    unordered_map< string , Transaction * > umap ;

    // filling umap 
    Create_row_vector_and_add_to_umap( fileName , umap ) ; 

    set<string> block_txns_set ; 
    vector<string> block_txns_vector ; 
    int block_weight = 0 ; 
    int block_fee = 0 ;

    cout << "umap size : " << umap.size() << endl ; 
    // Sort the umap as per the fee to weight ratio 
    set< pair< float , Transaction * > , greater< pair< float , Transaction * > > > sorted_txn_pair ; 
    for( auto p : umap ) 
        sorted_txn_pair.insert( {(float)p.second->fee / (float)p.second->weight, p.second } ) ; 

    // for( int i = 0 ; i < umap.size() ; i++ )
    // {
    //     sorted_txn_pair.insert( {(float)umap[i].second->fee / (float)umap[i].second->weight , umap[i].second } ) ; 
    // }

    // traverse the sorted set & push into block 
    while( !sorted_txn_pair.empty() && block_weight < max_wt ) 
    {
        bool found = false ; 
        for( auto itr = sorted_txn_pair.begin() ; itr != sorted_txn_pair.end() ; itr++ )
        {
            Transaction *txn_ptr = (*itr).second ; 
            
            string tx_id = txn_ptr->tx_id ; 
            int weight = txn_ptr->weight ; 
            int fee = txn_ptr->fee ; 
            if( allow_txn(txn_ptr , block_txns_set ) && block_weight + weight <= max_wt )
            {
                block_txns_vector.push_back(tx_id) ; 
                block_txns_set.insert(tx_id) ; 
                block_fee += fee ; 
                block_weight += weight ; 

                // erasing the txn from the Mempool(sorted_txn_pair )as it is included in the block
                sorted_txn_pair.erase(itr) ;
                found = true ; 
                break ; // haven't broke the loop therefore created the never ending loop here 
            }
        }
        if( !found ) 
            break ;
        // left this condition within the nested loop therefore as the block_weight < max_wt it created a never ending loop 

    }

    block_write( "block9.txt", block_txns_vector ) ; 

    cout << "Total Block fee : " << block_fee << endl ;
    cout << "Total Block weight : " << block_weight << endl ;
    cout << "Percentage of Block weight occupied : " << ((float)block_weight / (float)max_wt) * 100 << "%" << endl ;
    cout << "Number of transactions included the Block : " << block_txns_vector.size() << endl ; 

}
