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
      vector<int>occurence_vector; 
      
      /* 
       this denotes from which parent this child was created 
       */
       SuffixTreeNode *parent_address;
       /*Number of characters encountered upto this node*/
       int number_of_characters; 
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
				 //activeNode->child[text[activeEdge]-BASE]->occurence_vector.push_back(pos);
				 
				 /*A new leaf edge is created in above line starting
				   from  an existng node (the current activeNode), and
                   if there is any internal node waiting for it's suffix
                   link get reset, point the suffix link from that last
                   internal node to current activeNode. Then set lastNewNode
				   to NULL indicating no more node waiting for suffix link
                   reset.*/
                   
                   if(lastNewNode != NULL) {
						lastNewNode->suffixLink = activeNode->child[text[activeEdge]-BASE];
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
						//b) From new node 
						//int length = edgeLength(split);
						//split->occurence_vector.push_back(pos-length); 
				        
				   
				     //New leaf coming out of new internal node
				     split->child[text[pos]-BASE] = newNode(pos,&leafEnd);
						//occurence vector for new created leaf 
						//split->child[text[pos]-BASE]->occurence_vector.push_back(pos-length); 
						split->child[text[pos]-BASE]->parent_address = split; 
						updateOccurenceVector(split->child[text[pos]-BASE],pos); //update of occurence vector to root
						
				     next->start += activeLength; 
				     split->child[text[next->start]-BASE] = next; 
				     next->parent_address = split; 
				     
				   
				     /*We got a new internal node here. If there is any
					  internal node created in last extensions of same
					  phase which is still waiting for it's suffix link
					  reset, do it now.*/
					  
					  if(lastNewNode != NULL) {
							/*suffixLink of lastNewNode points to current newly
							  created internal node*/
							lastNewNode->suffixLink = split;
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
		/*for(int i='a';i<='e';i++){
			char ch = i;
			weight[ch] = 1;
		}*/
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

