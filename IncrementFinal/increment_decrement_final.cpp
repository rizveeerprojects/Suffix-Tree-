#include <bits/stdc++.h>
using namespace std;
#define MAX_CHAR 258
#define BASE 0 

/* Max_CHAR is number of characters possible 
 * Base is the central character 
 * 
 * */

struct SuffixTreeNode{
	//edge 
	struct SuffixTreeNode *child[MAX_CHAR+5]; 
	 
	//pointer to other node via suffix link
	struct SuffixTreeNode *suffixLink;
	
	/*(start, end) interval specifies the edge, by which the
     node is connected to its parent node. Each edge will
     connect two nodes,  one parent and one child, and
     (start, end) interval of a given edge  will be stored
     in the child node. Lets say there are two nods A and B
     connected by an edge with indices (5, 8) then this
     indices (5, 8) will be stored in node B. */
     
     int start;
     int *end;
     
     /*for leaf nodes, it stores the index of suffix for
      the path  from root to leaf*/
      int suffixIndex; 
      
     /*Occurence vector for an edge. Which specifies 
       the occurences of a string represented by an
       edge. suppose an edge represents "xyz" and occurence
       vector for this edge is {0,5,6} so, "xyz" occures from
       0,5 and 6th position
      */
      /* deque is used for pushing in front and to delete from end
       * */ 
      deque<int>occurence_vector;
      
      /* 
       this denotes from which parent this child was created 
       */
       SuffixTreeNode *parent_address;
       /*Number of characters encountered upto this node*/
       int number_of_characters; 
       
       /* node which is carrying suffix link to this node */
       SuffixTreeNode *reverseSuffixLink;
 };
 
 
 //text represents our input string 
 string text; 
 
 //Pointer to root node
 SuffixTreeNode *root = NULL;  
 
 /*lastNewNode will point to newly created internal node,
  waiting for it's suffix link to be set, which might get
  a new suffix link (other than root) in next extension of
  same phase. lastNewNode will be set to NULL when last
  newly created internal node (if there is any) got it's
  suffix link reset to new internal node created in next
  extension of same phase. */
  SuffixTreeNode *lastNewNode = NULL;
  //activeNode represents our node based on which we are working 
  SuffixTreeNode *activeNode = NULL; 
  
  /*activeEdge is represeted as input string character
  index (not the character itself)*/
  int activeEdge = -1;
  int activeLength = 0;
  
   /*remainingSuffixCount tells how many suffixes yet to
     be added in tree
   */
   int remainingSuffixCount = 0;
   int leafEnd = -1;
   int *rootEnd = NULL;
   int *splitEnd = NULL;
   
   /*For Incremental database*/
   SuffixTreeNode * incremental_activeNode; //the node which was main before encountering $
   int incremental_activeEdge; //before $, the activeEdge
   int incremental_activeLength;//before $, the activeLength
   int incremental_remainingSuffixCount;//before $, remainingSuffixCount
   int incremental_leafEnd;// before $, remaining leafEnd 
   struct Incremental_DB{
	 SuffixTreeNode *split_node;
	 SuffixTreeNode *successor;   
	 int activeEdge; //identify parentEdge
	 int successorEdge; //identify succesor 
	 int condition; //this says about the type of successor while included
	 /* 
	  * 1) successor is null, not split node case 
	  * 2) successor is present 
	  * */
   };
   
   int leafHarai; //to represent how may leafs are lost without '$'
   
   /*File controller*/
   FILE *fp;
   
   vector<Incremental_DB*>node_to_delete; //this nodes need to be deleted for incremental db
   
    void initialize(){
		root = NULL;
		lastNewNode = NULL;
		activeNode = NULL;
		
		activeEdge = -1;
		activeLength = 0; 
		
		remainingSuffixCount = 0;
		leafEnd = -1;
		rootEnd = NULL;
		splitEnd = NULL; 
   }
   
   
   /* This function creates a new SuffixTreeNode and returns
      it.Parameters are int start, int *end. end is 
      a pointer, so that one change can effect all the others
    */
   struct SuffixTreeNode *newNode(int start, int *end){
	   SuffixTreeNode *node = new SuffixTreeNode();
	   for(int i=0;i<MAX_CHAR;i++){
			node->child[i] = NULL; 
	   }
	   /*For root node, suffixLink will be set to NULL
		 For internal nodes, suffixLink will be set to root
		 by default in  current extension and may change in
		 next extension*/
		 
		node->suffixLink = root; 
		node->start = start;
		node->end = end;
		
		/*suffixIndex will be set to -1 by default and
		  actual suffix index will be set later for leaves
		  at the end of all phases*/
		  
		node->suffixIndex = -1;
		node->parent_address=NULL; //parent address = NULL 
		node->number_of_characters = 0; //number of characters upto this node 
		node->reverseSuffixLink=root; //reverse suffix link to express which node is connected with node by suffix link
		
		return node; 
   }
   
   
   /* Function to detect edgeLength
    */
   int edgeLength(SuffixTreeNode *node){
	 return *(node->end) - (node->start) + 1; 
   }
   
   int walkDown(SuffixTreeNode *node){
	  /*activePoint change for walk down (APCFWD) using
	    Skip/Count Trick  (Trick 1). If activeLength is greater
	    than current edge length, set next  internal node as
		activeNode and adjust activeEdge and activeLength
		accordingly to represent same activePoint*/
			
	  /* 
		node is the choice of next node  
	  */
	  if(activeLength >= edgeLength(node)) {
		  activeEdge += edgeLength(node);
		  activeLength -= edgeLength(node);
		  activeNode = node;
		  return 1; // we have gone downwards
      }
	  return 0; 
   }
   
   
    void extendSuffixTree(int pos){
		/*Extension Rule 1, this takes care of extending all
		  leaves created so far in tree*/	  
		  leafEnd = pos; 
		  
		  /* 
		   * This flag represents if $ is encountered or not 
		   * */
		   bool $_flag=false;
		   
		   if(text[pos] == '$') {
			/*
			 * saving everything for incremental database 
			 * */
			 incremental_activeNode = activeNode;
			 incremental_activeLength = activeLength;
			 incremental_activeEdge = activeEdge;
			 incremental_remainingSuffixCount=remainingSuffixCount;
			 $_flag=true;
		  }
		  
		  /*Increment remainingSuffixCount indicating that a
		  new suffix added to the list of suffixes yet to be
		  added in tree*/
		  remainingSuffixCount++;
		 
		/*set lastNewNode to NULL while starting a new phase,
		  indicating there is no internal node waiting for
		  it's suffix link reset in current phase*/
		  lastNewNode = NULL;
		
		 //Add all suffixes (yet to be added) one by one in tree
		  while(remainingSuffixCount>0){
				if(activeLength == 0) {
					//so try from root , active edge will be this character
					activeEdge = pos;
				}
				// There is no outgoing edge starting with activeEdge from activeNode
				 if(activeNode->child[text[activeEdge]-BASE] == NULL) {
					 //Extension Rule 2 (A new leaf edge gets created)
					 activeNode->child[text[activeEdge]-BASE] = newNode(pos,&leafEnd);
					 //parent update 
					 activeNode->child[text[activeEdge]-BASE]->parent_address = activeNode;
					 
					 //checking of '$'
					if($_flag) {
						//dollar found this node needs to be deleted for incremental db
						Incremental_DB *temp_here = new Incremental_DB();
						temp_here->split_node= activeNode->child[text[activeEdge]-BASE];
						temp_here->successor = NULL;
						temp_here->activeEdge = activeEdge;
						temp_here->condition = 1; //not split node case 
						temp_here->successorEdge = -1; //special for '$'
						node_to_delete.push_back(temp_here);
					}
					
					/*A new leaf edge is created in above line starting
				   from  an existng node (the current activeNode), and
                   if there is any internal node waiting for it's suffix
                   link get reset, point the suffix link from that last
                   internal node to current activeNode. Then set lastNewNode
				   to NULL indicating no more node waiting for suffix link
                   reset.*/
                   
                   if(lastNewNode != NULL) {
						lastNewNode->suffixLink = activeNode;
						//reverse suffix link 
						activeNode->reverseSuffixLink = lastNewNode; 
						lastNewNode = NULL;
				   } 
				 }
				 // There is an outgoing edge starting with activeEdge from activeNode
				 else {
					//Get the next node at the end of edge starting with activeEdge
					struct SuffixTreeNode *next = activeNode->child[text[activeEdge]-BASE]; 
					if(walkDown(next)) {
					 //Start from next node (the new activeNode)
					  continue; 
					}
					/*Extension Rule 3 (current character being processed
					  is already on the edge)*/
					if(text[next->start + activeLength] == text[pos]) {
					/*If a newly created node waiting for it's 
                     suffix link to be set, then set suffix link 
                     of that waiting node to curent active node */
                     if(lastNewNode != NULL && activeNode != root){
						lastNewNode->suffixLink = activeNode;
						//reverse suffix link
						activeNode->reverseSuffixLink = lastNewNode;
						
						lastNewNode = NULL; 
					 }
					  activeLength++;
					 /*STOP all further processing in this phase
					   and move on to next phase*/
					   break; 
					
				 }
				  /*We will be here when activePoint is in middle of
				    the edge being traversed and current character
                    being processed is not  on the edge (we fall off
                    the tree). In this case, we add a new internal node
                    and a new leaf edge going out of that new node. This
                    is Extension Rule 2, where a new leaf edge and a new
				    internal node get created*/
				    
				     splitEnd = new int();
				    *splitEnd = next->start + activeLength - 1;
				    
				    //New internal node
				    struct SuffixTreeNode * split = newNode(next->start,splitEnd);
				    activeNode->child[text[activeEdge]-BASE] = split; 
				    //parent update 
				    split->parent_address = activeNode;
				    
				   //New leaf coming out of new internal node
				     split->child[text[pos]-BASE] = newNode(pos,&leafEnd);
				     split->child[text[pos]-BASE]->parent_address = split;
				     
				     next->start += activeLength; 
				     split->child[text[next->start]-BASE] = next; 
				     next->parent_address = split; 
				     
				      /*  
				      * If '$' is encountered, we need to save split,it's child,activeEdge
				      * because here activeNode is parent of split so edge will come from
				      * parent of split to child with that activeEdge
				      */ 
				      if($_flag) {
						Incremental_DB *temp = new Incremental_DB();
						temp->split_node = split;
						temp->successor = next;
						temp->activeEdge=activeEdge;
						temp->condition = 2;
						temp->successorEdge = next->start;
						node_to_delete.push_back(temp);
					  }
					  
					   /*We got a new internal node here. If there is any
					  internal node created in last extensions of same
					  phase which is still waiting for it's suffix link
					  reset, do it now.*/
					  
					   if(lastNewNode != NULL) {
							/*suffixLink of lastNewNode points to current newly
							  created internal node*/
							lastNewNode->suffixLink = split;
							//reverse suffix link
							split->reverseSuffixLink = lastNewNode;
					  }
					  
					  /*Make the current newly created internal node waiting
						for it's suffix link reset (which is pointing to root
						at present). If we come across any other internal node
						(existing or newly created) in next extension of same
						phase, when a new leaf edge gets added (i.e. when
						Extension Rule 2 applies is any of the next extension
						of same phase) at that point, suffixLink of this node
						will point to that internal node.*/
						
						lastNewNode = split;
				     
		  }
		   /* One suffix got added in tree, decrement the count of
				suffixes yet to be added.*/
				remainingSuffixCount--;
				if(activeNode == root && activeLength>0) {
					activeLength--;
					activeEdge = pos - remainingSuffixCount + 1;
				}
				else if(activeNode != root) {
					activeNode = activeNode->suffixLink; 
				}
	}
}



void increment_tree(){
	
	//initialize 
	leafHarai = 0;
	for(int i=node_to_delete.size()-1;i>=0;i--){
		
		//debug: cout<<"i = " << i <<" "<< node_to_delete[i]->split_node->start<<" "<<node_to_delete[i]->condition<<endl;
		
		Incremental_DB *temp = node_to_delete[i];
		if(temp->condition == 1) {
			//debug: cout<<"hu"<<endl;
			//condition 1
			//direct edge connected 
			if(temp->split_node == NULL) continue; //not valid 
			if(temp->split_node->parent_address == NULL) continue; //not valid 
			
			leafHarai++; //logically one leaf gets lost
			
			//basically direct '$' connection
			temp->split_node->parent_address->child[text[activeEdge]-BASE] = NULL; //edge deleted
			int cnt=0;
			int save;
			for(int j=0;j<MAX_CHAR;j++){
				if(temp->split_node->parent_address->child[j] != NULL) {
					cnt++;
					if(cnt == 1) {
						save = j;
					}
				}
				if(cnt>1) break;
			}
			if(cnt==0){
				//its a leaf 
				if(temp->split_node->parent_address != root) {
					//develop leaf end
					int length = edgeLength(temp->split_node->parent_address);
					temp->split_node->parent_address->end = &leafEnd;
					temp->split_node->parent_address->start = leafEnd-length+1;
					
					if(temp->split_node->parent_address->reverseSuffixLink != NULL) {
						//this has become a leaf node now 
						//so it can't be an suffixLink Node 
						temp->split_node->parent_address->reverseSuffixLink->suffixLink = root;
					}
					temp->split_node->parent_address->suffixLink = root;  //this is already a leaf Node
					
					//a leaf Node created 
					leafHarai--;
				}
			}
			else if(cnt == 1) {
				//need to merge 
				if(temp->split_node->parent_address != root) {
					
					int length1 = edgeLength(temp->split_node->parent_address);
					int length2 = edgeLength(temp->split_node->parent_address->child[save]);
					
					int ch = temp->split_node->parent_address->start;
					//edge correction
					temp->split_node->parent_address->parent_address->child[text[ch]-BASE] = temp->split_node->parent_address->child[save];
					//parent correction ? ? 
					temp->split_node->parent_address->child[save]->parent_address = temp->split_node->parent_address->parent_address;
					//start correction
					temp->split_node->parent_address->child[save]->start =  *(temp->split_node->parent_address->child[save]->end) - (length1+length2)+1;
					
					//suffixLink correction
					if(temp->split_node->parent_address->reverseSuffixLink != NULL){
						//as this node doesn't exist, this can't be considered as suffix Link any more 
						temp->split_node->parent_address->reverseSuffixLink->suffixLink = root; 
					}
					//node deletion
					temp->split_node->parent_address->parent_address = NULL; //deleted 
				}
				
			}
			else {
				//nothing to merge this is not leaf 
			}
			//delete split_node
			temp->split_node->parent_address = NULL; 
		}
		else {
			//condition 2
			//split nodes 
			if(temp->split_node == NULL || temp->split_node->parent_address == NULL) continue; //invalid node 
			leafHarai++;
			
			if(temp->successor->parent_address == NULL) {
				//debug: cout<<"succesor nai "<<endl;
				//successor edge is deleted 
				temp->split_node->child['$'-BASE] = NULL; //'$' edge deleted
				//need to check if it has become an end node or not 
				int cnt=0;
				int save;
				for(int j=0;j<MAX_CHAR;j++){
					if(temp->split_node->child[j] != NULL) {
						cnt++;
						if(cnt == 1) {
							save = j;
						}
					}
					if(cnt>1) {
						break;
					}
				}
				if(cnt == 0) {
					//debug: cout<<"dhuke***"<<endl;
					//it has become a leaf node
					if(temp->split_node != root) {
						int length = edgeLength(temp->split_node);
						//debug: cout<<"length = " << length<<" "<<leafEnd<<" "<<temp->split_node->start<<" "<<*(temp->split_node->end)<<endl;
						temp->split_node->end = &leafEnd;
						temp->split_node->start = *(temp->split_node->end)-length+1;
						//debug: cout<<"length = " << length<<" "<<leafEnd<<" "<<temp->split_node->start<<" "<<*(temp->split_node->end)<<endl;
						if(temp->split_node->reverseSuffixLink != NULL) {
							temp->split_node->reverseSuffixLink->suffixLink=root; 
						}
						temp->split_node->suffixLink = root; //its now a leaf node  
						leafHarai--; 
					}
				}
				else if(cnt == 1) {
					//need to merge
					if(temp->split_node != root) {
						//edge creation
						int ch = temp->split_node->start;
						int length1 = edgeLength(temp->split_node);
						int length2 = edgeLength(temp->split_node->child[save]);
						//start correction
						temp->split_node->child[save]->start = *(temp->split_node->child[save]->end) - (length1+length2)+1;
						//parent correction
						temp->split_node->child[save]->parent_address=temp->split_node->parent_address;
						//edge creation
						temp->split_node->parent_address->child[text[ch]-BASE] = temp->split_node->child[save];
						
						//suffix Link correction
						if(temp->split_node->reverseSuffixLink != NULL) {
							temp->split_node->reverseSuffixLink->suffixLink=root; 
						} 
						
						//delete node
						temp->split_node->parent_address = NULL;
					}
				}
				else {
					//no need to merge 
					//no need to change end of this node 
				}
				
			}
			//everythings normal
			else if(temp->successor->parent_address != NULL) {
				//debug: cout<<"succesor ache "<<endl;
				if(temp->split_node != root) {
					int length1 = edgeLength(temp->split_node);
					int length2 = edgeLength(temp->successor); 
					int ch = temp->split_node->start;
				    //start correction
				    temp->successor->start = *(temp->successor->end) - (length1+length2) + 1;
				    //debug: cout<<"start hoi = " << temp->successor->start<<" "<<*(temp->successor->end)<<endl;
				    //parent correction
				    temp->successor->parent_address = temp->split_node->parent_address;
				    //edge creation
				    temp->split_node->parent_address->child[text[ch]-BASE] = temp->successor;
				    //suffix Link correction
				    if(temp->split_node->reverseSuffixLink != NULL) {
						temp->split_node->reverseSuffixLink->suffixLink=root; 
					}
					//node delete
					temp->split_node->parent_address=NULL;	
				}
			}
		}
	}
	activeNode = incremental_activeNode;
	activeLength = incremental_activeLength;
	activeEdge = incremental_activeEdge;
	remainingSuffixCount = incremental_remainingSuffixCount;
	node_to_delete.clear();
}

void printAllSuffix(SuffixTreeNode *node,vector<char>V){
		
		
		bool okay = false;
		for(int i=0;i<=MAX_CHAR;i++){
			if(node->child[i] != NULL) {
				okay=true;
				char ch=i;
				//cout<<"i = " << i << " "<<ch<<endl;
				//cout<<"start = " << node->child[i]->start<<" "<< "end = " << *(node->child[i]->end)<<" "<<endl;
				string s="";
				for(int j=node->child[i]->start; j<=*(node->child[i]->end);j++) {
					s=s+text[j];
					V.push_back(text[j]);
				}
				//cout<<"edge " << s<< endl;
				char arr[100000];
				for(int j=0;j<(int)s.size();j++){
					arr[j]=s[j];
				}
				arr[s.size()]='\0'; 
				//fprintf(fp,"%s\n",arr); //printing in file
				printAllSuffix(node->child[i],V);
				for(int j=node->child[i]->start; j<=*(node->child[i]->end);j++) {
					V.pop_back();
				}
			}
		}
		if(!okay) {
			//cout<<"pass complete" <<endl;
			for(int i=0;i<(int)V.size();i++) {
				cout<<V[i];
				fprintf(fp,"%c",V[i]);
			}
			fprintf(fp,"\n");
			//cout<<endl;
		}
		return;
   }
   
 
   
    //function to build suffix tree 
    void buildSuffixTree(){
		rootEnd = new int();
		*rootEnd = -1;
		
		root = newNode(-1,rootEnd);
		activeNode = root;
		for(int i=0;i<(int)text.size();i++){
			extendSuffixTree(i); 
		}
		/*cout<<"generating initial suffix"<<endl;
		vector<char>V;
		//printing all suffix 
		printAllSuffix(root,V);*/
	   
    }
    
    
    void decrement_tree(int pos){
		SuffixTreeNode *temp = root;
		//identify the branch
		while(true){
			SuffixTreeNode *notun = temp->child[text[pos]-BASE];
			int new_pos = pos+edgeLength(temp->child[text[pos]-BASE]);
			//cout<<"new_pos " << new_pos<<" "<<pos<<endl;

			if(new_pos==(int)text.size()) {
				//this leaf node needs to be deleted
				temp=notun;
				break;
			}
			temp=notun;
			pos=new_pos;
		}
		
		int len = edgeLength(temp);
		if(len == 1 && text[temp->start - BASE] == '$') {
			//$ edge
			if(temp->parent_address == root) {
				//should handle 
				//just '$' suffix 
			}
			else {
				//basically ab-$ means I need to delete ab
				temp->parent_address->child['$']=NULL; //deleting edge 
				SuffixTreeNode *intermediate = temp;
				temp=temp->parent_address; //basically need to delete parent
				intermediate->parent_address=NULL; //deleting parent
			}
		} 
		
		//debug: cout<<"done "<<temp->start<<" "<<*(temp->end)<<endl;
		//delete branch and modify stuffs
		temp->parent_address->child[text[temp->start]-BASE]=NULL; //edge delete

		//modify node
		bool $_exist=false;
		int cnt = 0;
		int save;
		for(int i=0;i<MAX_CHAR;i++){
			if(temp->parent_address->child[i] != NULL) {
				cnt++;
				if(i == '$') {
					$_exist=true;
					break;
				}
				if(cnt == 1){
                    save = i;
				}
			}
		}
		if($_exist) {
			//nothing to do
		}
		else {
			if(cnt==0){
				//needs to add the leafEnd
				if(temp->parent_address!= root) {
					int length = edgeLength(temp->parent_address);
					//end correction
					temp->parent_address->end= &leafEnd;
					//start correction
					temp->parent_address->start = leafEnd - length + 1;
					//suffix Link correction
					if(temp->parent_address->reverseSuffixLink != NULL) {
                            //this has become a leaf
                            temp->parent_address->reverseSuffixLink->suffixLink = root;
					}

				}

			}
			else if(cnt == 1) {
				//only one branch and without $
                if(temp->parent_address != root){
                    int length1 = edgeLength(temp->parent_address);
                    int length2 = edgeLength(temp->parent_address->child[save]);
                    int ch = temp->parent_address->start;

                    //modify start
                    temp->parent_address->child[save]->start = *(temp->parent_address->child[save]->end) - (length1+length2) + 1;
                    //modify parent
                    temp->parent_address->child[save]->parent_address=temp->parent_address->parent_address;
                    //edge creation
                    temp->parent_address->parent_address->child[text[ch]-BASE] = temp->parent_address->child[save];
                    //suffix Link modification
                    if(temp->parent_address->reverseSuffixLink != NULL) {
                        //this node doesn't belong so suffix Link can't point there
                        temp->parent_address->reverseSuffixLink->suffixLink = root;
                    }
                    temp->parent_address->parent_address=NULL; //parent lost node lost 
                }
			}
			else {
				//more than one branch
				//this node is fine
			}
		}
		temp->parent_address=NULL; //node delete
	}
	
	//function to find the string of suffix is present or not 
	int suffix_existence(int pos){
		
		SuffixTreeNode *temp = root;
		while(true){
			//debug: cout<<"choltese " << pos << " "<< temp->child[text[pos]-BASE]->start<<" "<<*(temp->child[text[pos]-BASE]->end)<<endl;
			SuffixTreeNode *notun = temp->child[text[pos]-BASE];
			if(notun == NULL) return 0; //node not created 
			bool ok=false;
			int fix=pos;
			for(int i=temp->child[text[fix]-BASE]->start;i<=*(temp->child[text[fix]-BASE]->end);i++){
				//debug: cout<<"i = " <<i << endl;
				if(pos<(int)text.size()-1) {
					if(text[pos] == text[i]) {
						//debug: cout<<"checking " << text[pos]<<" "<<text[i]<<endl;
						pos++;
						ok=true;
					}
					else {
						//mismatch in edge
						ok=false;
						return 0;
					} 
				}
				else {
					//edge didn't finish but string finished
					ok=false;
					return 0;
				}
			}
			//debug: cout<<"baire " << pos << endl;
			if(ok) {
				if(pos>(int)(text.size()-2)) {
					//full milse return 1
					return 1;
				}
				else {
					temp=temp->child[text[fix]-BASE];
				}
			}
		}
		return 0;
	}
	
	
	//function to find the largest unmatched string
	int find_largest_unmatched_string(int st,int en){
		int mid;
		int save;
		bool found=false;
		while(true){
			if(st>en) break;
			mid=(st+en)/2;
			int ans = suffix_existence(mid);
			if(ans==1) {
				//this exists
				st=mid+1;
			}
			else if(ans==0){
				en=mid-1; 
				found=true;
				save=mid;
			}
		}
		if(found) {
			return save;
		}
		return -1; //all exists 
	}

	void initialize_for_new_insertion(int got){
		if(got == -1) {
			//all suffixes are present
			//move from root
			activeNode = root; 
			activeLength = 0;
			activeEdge = -1; 
			remainingSuffixCount = 0;
		}
		else {
			int pos=got;
			remainingSuffixCount = text.size()-2 - got+1;
			SuffixTreeNode *temp = root; 
			while(true){
				activeNode = temp;
				activeLength = 0;
				activeEdge = pos;
				int fix=pos;
				bool ok=false;
				if(temp->child[text[fix]-BASE] == NULL) return;
				//cout<<temp->child[text[fix]-BASE]->start<<endl;
				for(int i=temp->child[text[fix]-BASE]->start;i<=*(temp->child[text[fix]-BASE]->end);i++){
					//cout<<text[i]<<endl;
					if(pos<=((int)text.size()-2) && text[i] == text[pos]) {
						pos++;
						activeLength++;
						ok=true;
					}
					else {
						ok=false;
						return;
					}
				}
				if(ok){
					temp=temp->child[text[fix]-BASE];
				}
				
			}
		}
	}
	
	void clear_occurence_vector(SuffixTreeNode *node){
		if(node == NULL) return;
		for(int i=0;i<MAX_CHAR;i++){
			if(node->child[i] != NULL) {
				clear_occurence_vector(node->child[i]);
			}
		}
		node->occurence_vector.clear();
		return;
	}
	
	deque<int> make_occurence_vector(SuffixTreeNode *node, int depth){
		bool move = false;
		for(int i=0;i<MAX_CHAR;i++){
			if(node->child[i] != NULL) {
				move=true;
				deque<int>temp;
				temp = make_occurence_vector(node->child[i],depth+edgeLength(node->child[i]));
				for(int j=0;j<(int)temp.size();j++){
					node->occurence_vector.push_back(temp[j]);
				}
			}
		}
		sort(node->occurence_vector.begin(),node->occurence_vector.end());
		if(move) return node->occurence_vector;
		else {
			node->occurence_vector.push_back(text.size()-depth);
			sort(node->occurence_vector.begin(),node->occurence_vector.end());
			return node->occurence_vector;
		}
	}
	
	void print_occurence_vector(SuffixTreeNode *node){
		
		for(int i=0;i<MAX_CHAR;i++){
			if(node->child[i] != NULL) {
				string s="";
				for(int j=node->child[i]->start;j<=*(node->child[i]->end);j++){
					s=s+text[j];
				}
				cout<<s<<endl;
				for(int j=0;j<(int)node->child[i]->occurence_vector.size();j++){
					cout<<node->child[i]->occurence_vector[j]<<" ";
				}
				cout<<endl;
				print_occurence_vector(node->child[i]);
			}
		}
		return;
	}
	
	
	void general_suffix_tree(){
		fp = fopen("general_input.txt","r");
		char arr[100000];
		fscanf(fp,"%s",arr);
		text="";
		for(int i=0;i<(int)strlen(arr);i++){
			text=text+arr[i];
		}
		fclose(fp);
		
		initialize();
		buildSuffixTree();
		fp = fopen("general_output.txt","w");
		vector<char>V;
		printAllSuffix(root,V);
		fclose(fp);
	}
	
	void test_increment(){
		fp = fopen("in.txt","r");
		char arr[100000];
		fscanf(fp,"%s",arr); //input
		fclose(fp);
		
		text="";
		for(int i=0;i<(int)strlen(arr);i++){
			text=text+arr[i]; //copying into text
		}
		
		//make normal suffix tree
		initialize();
		buildSuffixTree();
		
		string temp="";
		
		//random string generation to increment
		int length=random()%20+1;
		for(int i=1;i<=length;i++){
			int v = rand()%26+'a';
			char ch=v;
			temp=temp+ch;
		}
		temp=temp+'$';
		
		increment_tree(); //function to delete '$' and extra nodes created for this
		leafHarai--;
		
		length=text.size();
		//modify text
		text[length-1]=temp[0]; //'$' replaced
		for(int i=1;i<(int)temp.size();i++){
			text=text+temp[i];
		}
		//add for new suffixes
		for(int i=length-1;i<(int)text.size();i++){
			extendSuffixTree(i); 
		}
		
		//printing for modified tree
		fp = fopen("general_input.txt","w");
		for(int i=0;i<(int)text.size();i++){
			arr[i]=text[i];
		}
		arr[text.size()]='\n';
		fprintf(fp,"%s\n",arr);
		fclose(fp);
		
		//printing the suffix
		fp = fopen("incremental_output.txt","w");
		vector<char>V;
		printAllSuffix(root,V);
		fclose(fp);	
	}
	
	void decrement_testing(){
		
		fp = fopen("in.txt","r");
		char arr[100000];
		fscanf(fp,"%s",arr);
		fclose(fp);
		text="";
		for(int i=0;i<(int)strlen(arr);i++){
			text=text+arr[i];
		}
		
		//make normal suffix tree
		initialize();
		buildSuffixTree();
		
		int length=text.size()-1;
		int pos=7;
		//int pos =rand()%length;
		cout<<pos<<endl;
		
		//deleting
		for(int i=0;i<=pos;i++){
			decrement_tree(i);
		}
		leafEnd--; //leafEnd will decrease by one
		increment_tree();//modifying by removing '$'
		leafHarai--; // becase $ node not mandatory
		
		cout<<"leafHarai = " << leafHarai << endl;
		
		//for testing purpose
		
		//writing string
		fp = fopen("general_input.txt","w");
		int j= -1;
		for(int i=pos+1;i<(int)text.size()-1;i++){
			j++;
			arr[j] = text[i];
		}
		j++;
		arr[j]='\0';
		fprintf(fp,"%s\n",arr);
		fclose(fp);
		
		//writing edge val
		fp = fopen("decrement_output","w");
		vector<char>V;
		printAllSuffix(root,V);
		fclose(fp);
	}
	
	
	void decrement_increment_testing(){
		fp = fopen("in.txt","r");
		char arr[100000];
		fscanf(fp,"%s",arr);
		fclose(fp);
		text="";
		for(int i=0;i<(int)strlen(arr);i++){
			text=text+arr[i];
		}
		
		//make normal suffix tree
		initialize();
		buildSuffixTree();
		
		int length=text.size()-1;
		int pos=0;
		pos =rand()%length;
		/*************/
		//pos=3;
		cout<<"pos = " << pos<<endl;
		//deleting
		for(int i=0;i<=pos;i++){
			decrement_tree(i);
		}
		leafEnd--; //leafEnd will decrease by one
		increment_tree();//modifying by removing '$'
		leafHarai--; 
		
		int got=find_largest_unmatched_string(pos+1,text.size()-2);
		int sp = text.size()-2 - leafHarai + 1;
		if(leafHarai == 0) sp= -1;
		cout<<"got = " << got<<" "<<leafHarai<<" "<<sp<<endl;
		/**************/
		//got=sp;
		initialize_for_new_insertion(sp);
		remainingSuffixCount = leafHarai;

		int choose = rand()%30+1;
		string temp="";
		for(int i=1;i<=choose;i++){
			int v = rand()%26+'a';
			char ch=v;
			temp=temp+ch;
		}
		cout<<"text = "<<text<<endl;
		cout<<"temp = "<<temp<<endl;
		temp=temp+'$';
		
		
		int save=text.size()-1; //where to start from
		text[text.size()-1]=temp[0];
		for(int i=1;i<(int)temp.size();i++){
			text=text+temp[i];
		}
		cout<<"text = " <<text<<endl;
		for(int i=save;i<(int)text.size();i++){
			extendSuffixTree(i);
		}
		
		//input saved
		fp=fopen("general_input.txt","w");
		int j=-1;
		for(int i=pos+1;i<(int)text.size();i++){
			j++;
			arr[j]=text[i];
		}
		j++;
		arr[j]='\0';
		fprintf(fp,"%s\n",arr);
		fclose(fp);
		//output saved
		fp=fopen("decrement_increment.txt","w");
		vector<char>V;
		printAllSuffix(root,V);
		fclose(fp);
	}
	
   


int main(){
	srand(time(NULL));
	/*initialize();
	cin>>text;
	cout<<"main string " << endl;
	buildSuffixTree();
	
	//checking incremental db
	/*cout<<"*************"<<endl;
	increment_tree();
	cout<<"aschi " << endl;
	int length=text.size();
	text[length-1]='a';
	text=text+"y"+'$';
	for(int i=length-1;i<(int)text.size();i++){
		extendSuffixTree(i);
	}
	cout<<"string "<<text<<endl;
	vector<char>V;
	printAllSuffix(root,V);
	cout<<"******"<<endl;*/
	//checking decremental db
	/*cout<<"**************"<<endl;
	decrement_tree(0);
	decrement_tree(1);
	decrement_tree(2);
	decrement_tree(3);
	leafEnd--;
	increment_tree();
	int got = find_largest_unmatched_string(4,text.size()-2);
	cout<<"got = " << got<<endl;
	initialize_for_new_insertion(got);
	int length = text.size()-1;
	text[length] = 'b';
	text=text+"a"+'$';
	for(int i=length;i<(int)text.size();i++){
		cout<<text[i]<<endl;
		extendSuffixTree(i);
	}
	/*cout<<"string "<<text<<endl;
	vector<char>V;
	printAllSuffix(root,V);
	decrement_tree(1);
	cout<<"***********"<<endl;
	cout<<"string "<<text<<endl;
	vector<char>V;
	printAllSuffix(root,V);
	cout<<"checking occurrence vector " << endl;
	clear_occurence_vector(root);
	make_occurence_vector(root,0);
	print_occurence_vector(root);*/
	
	/*********************************************/
	//test_increment();
	/*********************************************/
	general_suffix_tree();
	/*********************************************/
	//decrement_testing();
	/********************************************/
	//decrement_increment_testing();
	
	return 0;
	
}
 
