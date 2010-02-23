
#include <shared/formats/castep.hpp>
#include <shared/nuclear_data.hpp>

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_ast.hpp>
#include <boost/spirit/include/classic_symbols.hpp>
#include <boost/spirit/include/classic_file_iterator.hpp>

#include <fstream>
#include <iostream>

using namespace BOOST_SPIRIT_CLASSIC_NS;

using namespace std;
using namespace SpinXML;

typedef file_iterator<char>                                               iter_t;
typedef tree_match<iter_t>                                                parse_tree_match_t;
typedef parse_tree_match_t::const_tree_iterator                           tree_iter_t;

//typedef ast_match_policy<iter_t>                                          match_policy_t;
//typedef scanner_policies<iteration_policy, match_policy_t, action_policy> scanner_policy_t;
//typedef scanner<iter_t, scanner_policy_t>                                 scanner_t;
typedef rule<>                                                            rule_t;


struct element_parser_type : symbols<unsigned> {
    /*
       Just for fun, try renaming this class to element. No need to
       change the element_p bit at the end. Go on, try it. When you
       run the program with the renamed class it segfaults.

       I have no idea what is going on (maybe something is being
       overwritten?).

       And no, I am not making this up.
    */
  element_parser_type() {
        add("H",1);
        add("C",6);
        add("F",9);
    }

} element_p;


struct sigma_parser_type : symbols<unsigned> {
    sigma_parser_type() {
        add("sigma_xx",1);
        add("sigma_yy",2);
        add("sigma_zz",3);
    }

} sigma_p;

struct v_parser_type : symbols<unsigned> {
    v_parser_type() {
        add("V_xx",1);
        add("V_yy",2);
        add("V_zz",3);
    }

} v_xyz_p;

struct castep : grammar<castep> {
    ///These IDs label all the places on interest in the parse tree
    enum {
        element_indexID=1                       ,
                                      
        headerID                                ,
        coord_blockID                           ,
        total_shielding_matrixID                ,
        total_matrixID                          ,
        eigensystem_block_total_shieldingID     ,
        eigensystem_block_totalID               ,
        anisotropy_blockID                      ,
        cq_eta_blockID                          ,
                                      
                                      
        eigen_block_total_shieldingID           ,
        eigen_block_totalID                     ,
                                      
        total_shielding_tensorID                ,
        total_tensorID                          ,
        quadrupole_blockID                      ,
                                      
        fileID                                  
    };
    
    void identify_spins(tree_iter_t tree) {
        //we are assuming we have been passed the root of the parse tree
        for(tree_iter_t j=tree->children.begin();j!=tree->children.end();++j) {
            if(j->value.id()==total_shielding_tensorID) {
                //Thanks to the grammar, we can rely on the child
                //nodes being present and correct
                
                //Header
                tree_iter_t header = j->children.begin();
                tree_iter_t atom   = header->children.begin();
                tree_iter_t index  = header->children.begin()+1;

                //Coordinates
                tree_iter_t coords = j->children.begin()+1;
                tree_iter_t x_coord = coords->children.begin();
                tree_iter_t y_coord = coords->children.begin()+1;
                tree_iter_t z_coord = coords->children.begin()+2;

                cout << "Creating a spin " <<
                    string(atom->value.begin(),atom->value.end()) << " " << 
                    string(index->value.begin(),index->value.end()) << " at (" << 
                    string(x_coord->value.begin(),x_coord->value.end()) << " " <<
                    string(y_coord->value.begin(),y_coord->value.end()) << " " <<
                    string(z_coord->value.begin(),z_coord->value.end()) << " " <<
                    ")" << endl;
            }
        }
    }

    void process_tree(tree_iter_t tree) {
        //First pass, create spin objects and add them to the spin system
        identify_spins(tree);
        //Second pass, assign interactions to those spins.
        /*if(tree->value.id()==fileID) {
            for(tree_iter_t j=tree->children.begin();j!=tree->children.end();++j) {
                process_tree(j);
            }
        } else if( tree->value.id()==total_shielding_tensorID            ) {
            
        } else if( tree->value.id()==total_tensorID                      ) {
        } else if( tree->value.id()==quadrupole_blockID                  ) {
        } else if( tree->value.id()==headerID                            ) {}
        else if( tree->value.id()==coord_blockID                       ) {}
        else if( tree->value.id()==total_shielding_matrixID            ) {}
        else if( tree->value.id()==total_matrixID                      ) {}
        else if( tree->value.id()==eigensystem_block_total_shieldingID ) {}
        else if( tree->value.id()==eigensystem_block_totalID           ) {}
        else if( tree->value.id()==anisotropy_blockID                  ) {}
        else if( tree->value.id()==cq_eta_blockID                      ) {}
                                                                                                     
        else if( tree->value.id()==eigen_block_total_shieldingID       ) {}
        else if( tree->value.id()==eigen_block_totalID                 ) {}
                                                                                                     
        else {
        }*/
    }
    void process_quadrupole() {

    }
    void print_tree(tree_iter_t tree,string indent="") {
        string nodename;
        if(      tree->value.id()==element_indexID                     ) {nodename="element_indexID";}
        else if( tree->value.id()==headerID                            ) {nodename="headerID";}
        else if( tree->value.id()==coord_blockID                       ) {nodename="coord_blockID";}
        else if( tree->value.id()==total_shielding_matrixID            ) {nodename="total_shielding_matrixID";}
        else if( tree->value.id()==total_matrixID                      ) {nodename="total_matrixID";}
        else if( tree->value.id()==eigensystem_block_total_shieldingID ) {nodename="eigensystem_block_total_shieldingID";}
        else if( tree->value.id()==eigensystem_block_totalID           ) {nodename="eigensystem_block_totalID";}
        else if( tree->value.id()==anisotropy_blockID                  ) {nodename="anisotropy_blockID";}
        else if( tree->value.id()==cq_eta_blockID                      ) {nodename="cq_eta_blockID";}
                                                                                                                        
        else if( tree->value.id()==eigen_block_total_shieldingID       ) {nodename="eigen_block_total_shieldingID";}
        else if( tree->value.id()==eigen_block_totalID                 ) {nodename="eigen_block_totalID";}
                                                                                                                        
        else if( tree->value.id()==total_shielding_tensorID            ) {nodename="total_shielding_tensorID";}
        else if( tree->value.id()==total_tensorID                      ) {nodename="total_tensorID";}
        else if( tree->value.id()==quadrupole_blockID                  ) {nodename="quadrupole_blockID";}
                                                                                                                        
        else if( tree->value.id()==fileID                              ) {nodename="fileID";}          
        else {
            nodename="unknown";
        }
        cout << indent << nodename << "(" 
             << string(tree->value.begin(), tree->value.end()) << ")"<< endl;
        for(tree_iter_t j=tree->children.begin();j!=tree->children.end();++j) {
            print_tree(j,indent+"   ");
        }
    }

    template <typename ScannerT>
    struct definition {
#define RULE(name) rule<ScannerT, parser_context<>,parser_tag<name##ID> > name;
        RULE(element_index                         );
                                                   
        RULE(header                                );
        RULE(coord_block                           );
        RULE(total_shielding_matrix                );
        RULE(total_matrix                          );
        RULE(eigensystem_block_total_shielding     );
        RULE(eigensystem_block_total               );
        RULE(anisotropy_block                      );
        RULE(cq_eta_block                          );
                                                   
                                                   
        RULE(eigen_block_total_shielding           );
        RULE(eigen_block_total                     );
                                                   
        RULE(total_shielding_tensor                );
        RULE(total_tensor                          );
        RULE(quadrupole_block                      );
#undef DECLARE_RULE
        rule<ScannerT, parser_context<>,parser_tag<fileID> > file;
        rule<ScannerT, parser_context<>,parser_tag<fileID> > const& start() {return file;}

        definition(castep const& self) {

            element_index =
                element_p >> int_p;

            header = 
                no_node_d[lexeme_d[+str_p("=")]] >>
                no_node_d[str_p("Atom:")] >> root_node_d[element_index] >>
                no_node_d[lexeme_d[+str_p("=")]];

            coord_block =
                no_node_d[element_index >> str_p("Coordinates")] >> real_p >> real_p >> real_p >> no_node_d[str_p("A")];

            total_shielding_matrix=no_node_d[str_p("TOTAL") >> "Shielding" >> str_p("Tensor")] >>
                real_p >> real_p >> real_p >>
                real_p >> real_p >> real_p >>
                real_p >> real_p >> real_p;

            total_matrix=no_node_d[str_p("TOTAL") >> str_p("tensor")] >>
                real_p >> real_p >> real_p >>
                real_p >> real_p >> real_p >>
                real_p >> real_p >> real_p;

            eigen_block_total_shielding =
                no_node_d[element_index >> str_p("Eigenvalue")]  >> sigma_p >> real_p >> no_node_d[str_p("(ppm)")] >>
                no_node_d[element_index >> str_p("Eigenvector")] >> sigma_p >> real_p >> real_p >> real_p;

            eigen_block_total =
                no_node_d[element_index >> str_p("Eigenvalue")]  >> v_xyz_p >> real_p >>
                no_node_d[element_index >> str_p("Eigenvector")] >> v_xyz_p >> real_p >> real_p >> real_p;


            eigensystem_block_total_shielding = eigen_block_total_shielding >> eigen_block_total_shielding >> eigen_block_total_shielding;
            eigensystem_block_total           = eigen_block_total           >> eigen_block_total           >> eigen_block_total;

            anisotropy_block =
                no_node_d[element_index] >> str_p("Isotropic:") >> real_p >> no_node_d[str_p("(ppm)")] >>
                no_node_d[element_index] >> str_p("Anisotropy:") >> real_p >> no_node_d[str_p("(ppm)")] >>
                no_node_d[element_index] >> str_p("Asymmetry:") >> real_p;

            cq_eta_block = 
                no_node_d[element_index >> str_p("Cq:")] >> real_p >> no_node_d[str_p("(MHz)")] >>
                no_node_d[element_index >> str_p("Eta:")] >> real_p;

            total_shielding_tensor = 
                header >>
                coord_block >>
                total_shielding_matrix >>
                eigensystem_block_total_shielding >>
                anisotropy_block;

            total_tensor = 
                header >>
                coord_block >>
                total_matrix >>
                eigensystem_block_total >>
                cq_eta_block;

            quadrupole_block = 
                no_node_d[lexeme_d[+str_p("=")]] >>
                no_node_d[str_p("Using") >> str_p("the") >> str_p("following") >> str_p("values") >>
                          str_p("of") >> str_p("the") >> str_p("electric") >> str_p("quadrupole") >>
                          str_p("moment")] >>
                no_node_d[element_p >> str_p("User") >> str_p("defined.") >> str_p("Q") >> str_p("=")] >> real_p >> no_node_d[str_p("Barn")] >>
                no_node_d[element_p >> str_p("User") >> str_p("defined.") >> str_p("Q") >> str_p("=")] >> real_p >> no_node_d[str_p("Barn")] >>
                no_node_d[lexeme_d[+str_p("=")]];
                

            file=*(
                   quadrupole_block |
                   total_shielding_tensor | 
                   total_tensor 
                   ) >> end_p;
        }
    };
}  castep_p;

void CASTEPLoader::LoadFile(SpinSystem* ss,const char* filename) const {

    ss->Clear();

    iter_t fin(filename);
    iter_t end=fin.make_end();
    if(!fin) {
        throw runtime_error("Couldn't open file");
    }

    tree_parse_info<iter_t> result=ast_parse(fin,end,castep_p,space_p);
    if(result.full) {
        castep_p.print_tree(result.trees.begin());
        castep_p.process_tree(result.trees.begin());
    } else {
        string badtext(result.stop,end);
        if(badtext.length() > 200) {
            badtext=badtext.substr(0,200);
        }
        string error="Syntax error near: \"" + badtext + "\"";
        cout << error << endl;
        throw runtime_error(error);
    }

    ss->sigReloaded();
}

