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
	 int activeEdge; //identify parentEdge
	 int successorEdge; //identify succesor
	 int condition; //this says about the type of successor while included
	 /*
	  * 1) successor is null, not split node case
	  * 2) successor is present
	  * */
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
				if(activeNode != root) {
					activeNode = activeNode->suffixLink;
				}
	}
}


void increment_tree(){
	for(int i=node_to_delete.size()-1;i>=0;i--){
		Incremental_DB *temp = node_to_delete[i];
		if(temp->condition == 1) {
			//condition 1
			//direct edge connected
			if(temp->split_node == NULL) continue; //not valid
			if(temp->split_node->parent_address == NULL) continue; //not valid
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
					//parent correction
					temp->split_node->parent_address->child[save]->parent_address = temp->split_node->parent_address->parent_address->child[text[ch]-BASE];
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
			if(temp->successor == NULL || temp->successor->parent_address == NULL) {
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
					//it has become a leaf node
					if(temp->split_node != root) {
						int length = edgeLength(temp->split_node);
						temp->split_node->end = &leafEnd;
						temp->split_node->start = *(temp->split_node->end)-length+1;
						if(temp->split_node->reverseSuffixLink != NULL) {
							temp->split_node->reverseSuffixLink->suffixLink=root;
						}
						temp->split_node->suffixLink = root; //its now a leaf node
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
			if(temp->successor != NULL  || temp->successor->parent_address != NULL) {

				if(temp->split_node != root) {
					int length1 = edgeLength(temp->split_node);
					int length2 = edgeLength(temp->successor);
					int ch = temp->split_node->start;
				    //start correction
				    temp->successor->start = *(temp->successor->end) - (length1+length2) + 1;
				    //parent correction
				    temp->successor->parent_address = temp->split_node->parent_address;
				    //edge creation
				    temp->split_node->parent_address->child[text[ch]-BASE] = temp->successor;
				    //suffix Link correction
				    if(temp->split_node->reverseSuffixLink != NULL) {
						temp->split_node->reverseSuffixLink->suffixLink=root;
					}
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
				//cout<<"i = " << i << endl;
				//cout<<"start = " << node->child[i]->start<<" "<< "end = " << *(node->child[i]->end)<<" "<<endl;
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

    }


    void decrement_tree(int pos){
		SuffixTreeNode *temp = root;
		//identify the branch
		while(true){
			SuffixTreeNode *notun = temp->child[text[pos]-BASE];
			int new_pos = pos+edgeLength(temp);

			if(new_pos==(int)text.size()) {
				//this leaf node needs to be deleted
				temp=notun;
				break;
			}
			temp=notun;
			pos=new_pos;
		}
		//delete branch and modify stuffs
		temp->parent_address->child[pos]=NULL; //edge delete

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
                }
			}
			else {
				//more than one branch
				//these node is fine
			}
		}

		temp->parent_address=NULL; //node delete


	}



int main(){
	initialize();
	cin>>text;
	cout<<"main string " << endl;
	buildSuffixTree();

	//checking incremental db
	cout<<"*************"<<endl;
	increment_tree();
	int length=text.size();
	text[length-1]='a';
	text=text+"abc"+'$';
	for(int i=length-1;i<(int)text.size();i++){
		extendSuffixTree(i);
	}
	cout<<"string "<<text<<endl;
	vector<char>V;
	printAllSuffix(root,V);
}
