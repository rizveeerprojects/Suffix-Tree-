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
 SuffixTreeNode *root;  
 
 /*lastNewNode will point to newly created internal node,
  waiting for it's suffix link to be set, which might get
  a new suffix link (other than root) in next extension of
  same phase. lastNewNode will be set to NULL when last
  newly created internal node (if there is any) got it's
  suffix link reset to new internal node created in next
  extension of same phase. */
  SuffixTreeNode *lastNewNode;
  //activeNode represents our node based on which we are working 
  SuffixTreeNode *activeNode; 
  
  /*activeEdge is represeted as input string character
  index (not the character itself)*/
  int activeEdge;
  int activeLength;
  
   /*remainingSuffixCount tells how many suffixes yet to
     be added in tree
   */
   int remainingSuffixCount;
   int leafEnd;
   int *rootEnd;
   int *splitEnd;
   
   
   /*For Incremental database*/
   SuffixTreeNode * incremental_activeNode; //the node which was main before encountering $
   int incremental_activeEdge; //before $, the activeEdge
   int incremental_activeLength;//before $, the activeLength
   int incremental_remainingSuffixCount;//before $, remainingSuffixCount
   int incremental_leafEnd;// before $, remaining leafEnd 
   struct Incremental_DB{
	 SuffixTreeNode *split_node;
	 SuffixTreeNode *successor;   
	 int activeEdge;
   };
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
   
   
   //function to update occurence vector upto parent 
   void updateOccurenceVector(SuffixTreeNode *node,int pos){
	   /* 
	    * This will create an entry in occurence vector upto root for a branch
	      moving will be through parent SuffixTreeNode 
	    */
	   
	   //number of characters from root encountered 
	   int charactersFromRoot = node->parent_address->number_of_characters;
	   int startOfSuffix = pos-charactersFromRoot;
	   SuffixTreeNode *temp=node;
	   while(true){
			if(temp==root) break;
			temp->occurence_vector.push_back(startOfSuffix);
			temp=temp->parent_address;
	   }
	   return;
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
				 activeNode->child[text[activeEdge]-BASE]->number_of_characters = activeNode->number_of_characters;
				 
				 //occurence vector initiated 
				 updateOccurenceVector(activeNode->child[text[activeEdge]-BASE],pos); 
				 
				 //checking of '$'
				 if($_flag) {
					//dollar found this node needs to be deleted for incremental db
					Incremental_DB *temp_here = new Incremental_DB();
					temp_here->split_node= activeNode->child[text[activeEdge]-BASE];
					temp_here->successor = NULL;
					temp_here->activeEdge = activeEdge;
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
						lastNewNode->suffixLink = activeNode->child[text[activeEdge]-BASE];
						//reverse suffix link 
						activeNode->child[text[activeEdge]-BASE]->reverseSuffixLink = lastNewNode; 
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
					 
					 /***********************************/
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
				    split->number_of_characters=activeNode->number_of_characters+activeLength; //every split node is kind of fixed node and they don't change upto them, number of characters 
						
						//occurence vector
						//a) From Next 
						for(int i=0;i<(int)next->occurence_vector.size();i++){
							split->occurence_vector.push_back(next->occurence_vector[i]);
						} 
						
				     //New leaf coming out of new internal node
				     split->child[text[pos]-BASE] = newNode(pos,&leafEnd);
						//occurence vector for new created leaf 
						//split->child[text[pos]-BASE]->occurence_vector.push_back(pos-length); 
						split->child[text[pos]-BASE]->parent_address = split; 
						updateOccurenceVector(split->child[text[pos]-BASE],pos); //update of occurence vector to root
						
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
				if(activeNode != root) {
					activeNode = activeNode->suffixLink; 
				}
		  }
   }
   
   void printAllSuffix(SuffixTreeNode *node,vector<char>V){
		
		bool okay = false;
		for(int i=0;i<=MAX_CHAR;i++){
			if(node->child[i] != NULL) {
				okay=true;
				for(int j=node->child[i]->start; j<=*(node->child[i]->end);j++) {
					V.push_back(text[j]);
				}
				printAllSuffix(node->child[i],V);
				for(int j=node->child[i]->start; j<=*(node->child[i]->end);j++) {
					V.pop_back();
				}
			}
		}
		if(!okay) {
			for(int i=0;i<(int)V.size();i++) {
				cout<<V[i];
			}
			cout<<endl;
		}
		return;
   }
   
   
   //function to clear memory 
   void clearingMemory(SuffixTreeNode *node){
		for(int i=0;i<=MAX_CHAR;i++){
			if(node->child[i] != NULL) {
				clearingMemory(node->child[i]);
			}
		}
		free(node);
		return;
   }
   
    //function to print occurence vector
   void print_occurence_vector(SuffixTreeNode *node){
		for(int i=0;i<=MAX_CHAR;i++){
			if(node->child[i] != NULL) {
				cout << "characters ";
				int st=node->child[i]->start;
				int en = *(node->child[i]->end);
				for(int j=st;j<=en;j++){
					cout<<text[j];
				}
				cout<<endl;
				for(int j=0;j<(int)node->child[i]->occurence_vector.size();j++){
					cout<<node->child[i]->occurence_vector[j]<<" ";
				}
				cout<<endl;
				print_occurence_vector(node->child[i]);
			}
		}
   } 
   
   
   //function to generate pattern 
   map<char,double>weight;
   double minimumSupportThreshhold=0.0;
   double maxW=1; 
   int max_skip=3;
   
   struct Patterns{
	 string s; //the string 
	 vector<int>occurence_vector; //occurence  
	 SuffixTreeNode *node;  //the node info
	 int seen; //upto seen in an edge
	 int skip_cnt; //character skipped 
	 double weight; //weight sum 
	 int len;   //length of string 
	 bool inserted;//inserted or not 
   };
   
   struct Frequent_Pattern{
	  string s; //the string 
	  vector<int>occurence_vector; //occurence  
   };
   
   vector<Frequent_Pattern>freq; 
   
   void generatePattern(SuffixTreeNode *node){
	 //init 
	 queue<Patterns*>Q; //pattern type and address 
	 for(int i=0;i<MAX_CHAR;i++){
		if(node->child[i] != NULL) {
			int support_p = node->child[i]->occurence_vector.size();
			if(support_p * maxW < minimumSupportThreshhold) continue; //condition 1 failed 
			Patterns *temp = new Patterns();
			char ch = i;
			temp->s=""; 
			temp->s += ch; 
			for(int j=0;j<(int)node->child[i]->occurence_vector.size();j++){
					temp->occurence_vector.push_back(node->child[i]->occurence_vector[j]);
			}
			temp->node = node->child[i];
			temp->seen = node->child[i]->start;
			temp->weight = weight[ch]; 
			temp->skip_cnt = 0;
			temp->len = 1; 
			temp->inserted = false;
			
			if(support_p * temp->weight >= minimumSupportThreshhold){
				Frequent_Pattern notun;
				notun.s="";
				notun.s += ch;
				for(int j=0;j<(int)node->child[i]->occurence_vector.size();j++){
					notun.occurence_vector.push_back(node->child[i]->occurence_vector[j]);
				}
				temp->inserted=true;
				freq.push_back(notun);
				
			} 
			Q.push(temp);
		}
	 }
	 //bfs  	
	 while(Q.empty() != true){
		Patterns *u = Q.front();
		Q.pop();
		if(u->seen == *(u->node->end)) {
				//this node traversing complete 
				//goto new child 
				bool ok=false;
				for(int i=0;i<MAX_CHAR;i++){
					if(u->node->child[i] != NULL) {
						Patterns *temp = new Patterns();
						temp->s=u->s;
						temp->node=u->node->child[i];
						temp->seen = -1; // -1 means fresh start 
						temp->weight=u->weight;
						temp->skip_cnt=u->skip_cnt;
						temp->len=u->len;
						for(int j=0;j<(int)u->occurence_vector.size();j++){
							temp->occurence_vector.push_back(u->occurence_vector[j]);
						}
						Q.push(temp);
						ok=true;
					}
				}
				if(!ok) {
					//no move ending done
					double w = u->weight/u->len;
					if(((w * u->occurence_vector.size()) >= minimumSupportThreshhold) && u->inserted == false){
						Frequent_Pattern notun;
						notun.s=u->s;
						for(int j=0;j<(int)u->occurence_vector.size();j++){
							notun.occurence_vector.push_back(u->occurence_vector[j]);
						}
						freq.push_back(notun);
						u->inserted=true;
					}
				}
		}
		else {
			if(u->seen == -1) {
				//fresh start from child 
				//without skip 
				char ch = text[u->node->start];
				Patterns *temp = new Patterns();
				temp->s=u->s+ch;
				temp->node=u->node;
				temp->seen=u->node->start;
				temp->weight=u->weight+weight[ch];
				temp->skip_cnt=0;
				temp->len=u->len+1;
				for(int j=0;j<(int)u->node->occurence_vector.size();j++){
					temp->occurence_vector.push_back(u->node->occurence_vector[j]);
				}
				if(temp->occurence_vector.size() * maxW >= minimumSupportThreshhold) {
					//condition 1 satisfied 
					temp->inserted=false;
					double w = temp->weight/temp->len;
					if(temp->occurence_vector.size() * w >= minimumSupportThreshhold) {
						Frequent_Pattern notun;
						notun.s = temp->s;
						for(int j=0;j<(int)temp->occurence_vector.size();j++){
							notun.occurence_vector.push_back(temp->occurence_vector[j]);
						}
						freq.push_back(notun);
						temp->inserted=true;
					}
					Q.push(temp);
				}
				else {
					free(temp);
				}
				//with skip
				if((temp->skip_cnt+1)<=max_skip){
					temp = new Patterns();
					temp->s=u->s+"*";
					temp->node=u->node;
					temp->seen=u->node->start;
					temp->weight=u->weight;
					temp->skip_cnt=u->skip_cnt+1;
					temp->len=u->len;
					for(int j=0;j<(int)u->node->occurence_vector.size();j++){
						temp->occurence_vector.push_back(u->node->occurence_vector[j]);
					}
					if(temp->occurence_vector.size() * maxW >= minimumSupportThreshhold) {
						temp->inserted=false;
						Q.push(temp);
					}
					else {
						free(temp);
					}
				}
			}
			else {
				//same edge traversing
				//without skipping
				Patterns *temp = new Patterns();
				char ch=text[u->seen+1];
				temp->s=u->s+ch;
				temp->node=u->node;
				temp->seen=u->seen+1;
				temp->weight=u->weight+weight[ch];
				temp->skip_cnt=0;
				temp->len=u->len+1;
				for(int j=0;j<(int)u->occurence_vector.size();j++){
					temp->occurence_vector.push_back(u->occurence_vector[j]);
				}
				temp->inserted=false;
				//check frequent or not
				double w = temp->weight/temp->len;
				if(temp->occurence_vector.size() * w >= minimumSupportThreshhold) {
					//frequent 
					Frequent_Pattern notun;
					notun.s = temp->s;
					for(int j=0;j<(int)temp->occurence_vector.size();j++){
						notun.occurence_vector.push_back(temp->occurence_vector[j]);
					}
					freq.push_back(notun);
					temp->inserted=true;
				}
				Q.push(temp);
				//skipping 
				if((u->skip_cnt+1)<=max_skip) {
					//skip is valid 
					temp = new Patterns();
					temp->s=u->s+"*";
					temp->node=u->node;
					temp->seen=u->seen+1;
					temp->weight=u->weight;
					temp->skip_cnt=u->skip_cnt+1;
					temp->len=u->len;
					for(int j=0;j<(int)u->occurence_vector.size();j++){
						temp->occurence_vector.push_back(u->occurence_vector[j]);
					}
					temp->inserted=false;
					Q.push(temp);
				}
			}
		} 
		free(u); 
	 }
	 cout<<"patterns " << endl;
	 cout<<freq.size()<<endl;
	 for(int i=0;i<(int)freq.size();i++){
		cout<<freq[i].s<<endl;
		/*for(int j=0;j<(int)freq[i].occurence_vector.size();j++){
			cout<<freq[i].occurence_vector[j]<<" ";
		}
		cout<<endl;*/
	 }
   }
   
   
   void initialize_for_incrementalDB(){
		//delete the interior nodes created for '$'
		cout<<"size "<<node_to_delete.size()<<endl;
		
		for(int i=0;i<(int)node_to_delete.size();i++){
			Incremental_DB *temp = node_to_delete[i];
			for(int j=0;j<(int)temp->split_node->occurence_vector.size();j++){
				cout<<temp->split_node->occurence_vector[j]<<" ";
			}
			cout<<endl;
			if(temp->successor == NULL) cout<<"YES"<<endl;
			else cout<<"NO"<<endl;
		}
		
		for(int i=node_to_delete.size()-1;i>=0;i--){
			Incremental_DB *temp = node_to_delete[i];
			cout<<"notun step " << endl;
			for(int j=0;j<(int)temp->split_node->occurence_vector.size();j++){
				cout<<temp->split_node->occurence_vector[j]<<" ";
			}
			cout<<endl;
			getchar();
			if(temp->successor == NULL) {
				cout<<"asi "<<endl;
				if(temp->split_node == NULL) continue; //deleted due to decrement feature
				if(temp->split_node->parent_address == NULL) continue; //parent doesn't exist
				
				temp->split_node->parent_address->child['$'] = NULL; //thsere will be no edge for $ from root
				cout<<"ei porjonto " << endl;
				getchar();
				
				SuffixTreeNode *traverse = temp->split_node->parent_address;
				while(true){
					cout<<"choltesi " << endl;
					if(traverse==root) break;
					if(traverse == NULL) break; 
					if(traverse->occurence_vector.size() != 0) {
						if(traverse->occurence_vector.size() == 1) {
							if(traverse->parent_address == root) {
								//this exists due to delete feature
								//so something is just new start because of lots of delete
								break;
							}
							else {
								traverse->occurence_vector.pop_back();
								traverse=traverse->parent_address;
							}
						}
						else {
							traverse->occurence_vector.pop_back();
							traverse=traverse->parent_address;
							//debug: cout<<"aschi " << endl;
							//debug: getchar();
						}
					}
				}
			}
			else {
				//split node case
				if(temp->split_node == NULL) {
						continue; //this space is null
				}
				if(temp->successor == NULL){
					//need to merge parent and split node
					*(temp->split_node->parent_address->end) = *(temp->split_node->end);
					temp->split_node->parent_address->child[text[temp->activeEdge]-BASE]=NULL;
				}
				
				if(temp->successor != NULL) {
					temp->successor->start=temp->split_node->start; //start updated 
					temp->successor->parent_address=temp->split_node->parent_address; //parent address update
					temp->split_node->parent_address->child[text[temp->activeEdge]-BASE] = temp->successor; //link created 
				}
				
				
				//now erasing the last index which was inserted due to $
				SuffixTreeNode *traverse = temp->split_node->parent_address;
				while(true){
					if(traverse==root) break;
					if(traverse == NULL) break;
					if(traverse->occurence_vector.size() != 0) {
						if(traverse->occurence_vector.size() == 1) {
							if(traverse->parent_address == root) {
								//this exists due to delete feature
								//so something is just new start because of lots of delete
								break;
							}
						}
						else {
							traverse->occurence_vector.pop_back();
							traverse=traverse->parent_address;
						}
					}
				}
				
			}
		}
		cout<<"hoiche " << endl;
		//initialize the global variables 
		activeNode = incremental_activeNode;
		activeLength = incremental_activeLength;
		activeEdge = incremental_activeEdge;
		remainingSuffixCount = incremental_remainingSuffixCount;
		
		//clearing
		for(int i=0;i<(int)node_to_delete.size();i++){
			if(node_to_delete[i]->split_node != NULL) {
				free(node_to_delete[i]->split_node);
			}
			if(node_to_delete[i] != NULL) {
				free(node_to_delete[i]);
			}
		}
		node_to_delete.clear();
	}
	
	
	//function to delete from suffix tree due to deletion 
	void deletion_from_suffix_tree(int pos){
		/*pos is being delete from suffix*/
		SuffixTreeNode *temp = root;
		vector<SuffixTreeNode *>go_down; //represents the address to be freed
		vector<int>V;
		
		while(true){
			cout<<"going " << " pos = " << pos << endl;
			cout<<temp->child[text[pos]-BASE]->start<<" "<<*(temp->child[text[pos]-BASE]->end)<<endl;
			//delete from start
			temp->child[text[pos]-BASE]->occurence_vector.pop_front();
			//move through edge
			int new_pos=pos+edgeLength(temp->child[text[pos]-BASE]);
			if(temp->child[text[pos]-BASE]->occurence_vector.size() == 0) {
				/*if occurence vector empty this string doesn't belong any more now the node which
				  was connected to it must remove its suffix link 
				 */
				if(temp->child[text[pos]-BASE]->reverseSuffixLink != NULL) {
					//if that exists then change
					temp->child[text[pos]-BASE]->reverseSuffixLink->suffixLink = root;
				}
				go_down.push_back(temp->child[text[pos]-BASE]);
				V.push_back(pos);
			}
			else {
				//occurence vector already modified
			}
			if(new_pos>=(int)text.size()) break;
			temp=temp->child[text[pos]-BASE];
			pos=new_pos;
		}
		for(int i=go_down.size()-1;i>=0;i--){
			//this branch needs to be pruned
			go_down[i]->parent_address->child[text[V[i]]-BASE]=NULL;
			/*int cnt=0;
			int save=-1;
			//debug: cout<<"base = " << go_down[i]->start<<" "<<*(go_down[i]->end)<<" pos = " << pos << endl;
			for(int j=0;j<MAX_CHAR;j++){
				if(go_down[i]->parent_address->child[j] != NULL) {
					cnt++;
					if(cnt==1) {
						save=j;
					}
				}
				if(cnt>1) break; //multiple node
			}
			if(cnt==1){
				//only one child from a node
				if(go_down[i]->parent_address == root) {
					//a direct edge from root nothing to do
				}
				else {
					//start from immediate parent
					//parent updated
					go_down[i]->parent_address->child[save]->parent_address=go_down[i]->parent_address->parent_address;
					
					//edge create
					int shuru = go_down[i]->parent_address->start;
					go_down[i]->parent_address->parent_address->child[text[shuru]-BASE]=go_down[i]->parent_address->child[save];
					free(go_down[i]->parent_address); //this node doesn't exist now
				}
			}*/
			free(go_down[i]);
		}
		
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
		vector<char>V;
		//printing all suffix 
		printAllSuffix(root,V);
		//print occurence vector
		print_occurence_vector(root);
		//generate pattern
		
		/* segment for testing incremental DB */
		/*****************************************************/
		/*
		cout<<"incremental DB example"<<endl;
		initialize_for_incrementalDB();
		int sz=text.size();
		text[text.size()-1]='b';
		text = text+'$';
		cout<<text<<endl;
		cout<<"came " << endl;
		for(int i=sz-1;i<(int)text.size();i++){
			extendSuffixTree(i);
		}
		cout<<"done"<<endl;
		//printing all suffix 
		//printAllSuffix(root,V);
		//print occurence vector
		print_occurence_vector(root);
		*/
		
		/*************************************/
		int pos = -1;
		while(true){
			int choice;
			scanf("%d",&choice);
			if(choice==0){
				break;
			}
			if(choice==1){
				cout<<"input increment " << endl;
				string s;
				cin>>s;
				int length = text.size();
				text[text.size()-1]=s[0];
				text=text+'$';
				initialize_for_incrementalDB();
				for(int i=length-1;i<(int)text.size();i++){
					extendSuffixTree(i);
				}
				cout<<"complete " << endl;
				cout<<"suffixes " << endl;
				printAllSuffix(root,V);
				cout<<"occurence vector " << endl;
				print_occurence_vector(root);
			}
			if(choice == 2) {
				if((pos+1)<=(text.size()-2-incremental_remainingSuffixCount)) {
					pos++;
					cout<<"deleting " << pos << endl;
					deletion_from_suffix_tree(pos);
					cout<<"suffixes " << endl;
					printAllSuffix(root,V);
					cout<<"occurence vector " << endl;
					print_occurence_vector(root);
				}
			}
		}
		
		
		
		weight['a']=0.8;
		weight['b']=0.6;
		weight['c']=0.7;
		weight['d']=0.5;
		weight['$']=0.0001;
		//generatePattern(root);
		
		//clearing memory 
		clearingMemory(root);
		
   }
  
  
int main(void){
	
	
	initialize(); 
	cin>>text;
	buildSuffixTree();
}


