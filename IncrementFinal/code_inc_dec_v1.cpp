#include <bits/stdc++.h>
using namespace std;
#define MAX_CHAR    258
#define MAX_L       100000
#define BASE        0

struct SuffixTreeNode {
    struct SuffixTreeNode *child[MAX_CHAR+5];
    struct SuffixTreeNode *suffixLink;
    int start;
    int *end;

    deque<int>occurence_vector;
    SuffixTreeNode *parent;
    /*Number of characters encountered upto this node*/
    int number_of_characters; /// updated while running the dfs
    SuffixTreeNode *reverseSuffixLink;
};


//text represents our input string
string text;

//Pointer to root node
SuffixTreeNode *root = NULL;

SuffixTreeNode *lastNewNode = NULL;
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
struct Incremental_DB
{
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

void initialize()
{
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
struct SuffixTreeNode *newNode(int start, int *end)
{
    SuffixTreeNode *node = new SuffixTreeNode();
    for(int i=0; i<MAX_CHAR; i++)
    {
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
    node->parent=NULL; //parent address = NULL
    node->number_of_characters = 0; //number of characters upto this node
    node->reverseSuffixLink=root; //reverse suffix link to express which node is connected with node by suffix link

    return node;
}


/* Function to detect edgeLength
 */
int edgeLength(SuffixTreeNode *node)
{
    return *(node->end) - (node->start) + 1;
}

int walkDown(SuffixTreeNode *node)
{
    /*activePoint change for walk down (APCFWD) using
      Skip/Count Trick  (Trick 1). If activeLength is greater
      than current edge length, set next  internal node as
    activeNode and adjust activeEdge and activeLength
    accordingly to represent same activePoint*/

    /*
    node is the choice of next node
    */
    if(activeLength >= edgeLength(node))
    {
        activeEdge += edgeLength(node);
        activeLength -= edgeLength(node);
        activeNode = node;
        return 1; // we have gone downwards
    }
    return 0;
}


void extendSuffixTree(int pos)
{
    /*Extension Rule 1, this takes care of extending all
      leaves created so far in tree*/
    leafEnd = pos;

    /*
     * This flag represents if $ is encountered or not
     * */
    bool $_flag=false;

    if(text[pos] == '$')
    {
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
    while(remainingSuffixCount>0)
    {
        if(activeLength == 0)
        {
            //so try from root , active edge will be this character
            activeEdge = pos;
        }
        // There is no outgoing edge starting with activeEdge from activeNode
        if(activeNode->child[text[activeEdge]-BASE] == NULL)
        {
            //Extension Rule 2 (A new leaf edge gets created)
            activeNode->child[text[activeEdge]-BASE] = newNode(pos,&leafEnd);
            //parent update
            activeNode->child[text[activeEdge]-BASE]->parent = activeNode;

            //checking of '$'
            if($_flag)
            {
                //dollar found this node needs to be deleted for incremental db
                Incremental_DB *temp_here = new Incremental_DB();
                temp_here->split_node= activeNode->child[text[activeEdge]-BASE];
                temp_here->successor = NULL;
                temp_here->activeEdge = activeEdge;
                temp_here->condition = 1; //not split node case
                temp_here->successorEdge = -1; //special for '$'
                node_to_delete.push_back(temp_here);
            }

            if(lastNewNode != NULL)
            {
                lastNewNode->suffixLink = activeNode;
                //reverse suffix link
                activeNode->reverseSuffixLink = lastNewNode;
                lastNewNode = NULL;
            }
        }
        // There is an outgoing edge starting with activeEdge from activeNode
        else
        {
            //Get the next node at the end of edge starting with activeEdge
            struct SuffixTreeNode *next = activeNode->child[text[activeEdge]-BASE];
            if(walkDown(next))
            {
                //Start from next node (the new activeNode)
                continue;
            }
            /*Extension Rule 3 (current character being processed
              is already on the edge)*/
            if(text[next->start + activeLength] == text[pos])
            {
                /*If a newly created node waiting for it's
                 suffix link to be set, then set suffix link
                 of that waiting node to curent active node */
                if(lastNewNode != NULL && activeNode != root)
                {
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
            split->parent = activeNode;

            //New leaf coming out of new internal node
            split->child[text[pos]-BASE] = newNode(pos,&leafEnd);
            split->child[text[pos]-BASE]->parent = split;

            next->start += activeLength;
            split->child[text[next->start]-BASE] = next;
            next->parent = split;

            /*
            * If '$' is encountered, we need to save split,it's child,activeEdge
            * because here activeNode is parent of split so edge will come from
            * parent of split to child with that activeEdge
            */
            if($_flag)
            {
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

            if(lastNewNode != NULL)
            {
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
        if(activeNode == root && activeLength>0)
        {
            activeLength--;
            activeEdge = pos - remainingSuffixCount + 1;
        }
        else if(activeNode != root)
        {
            activeNode = activeNode->suffixLink;
        }
    }
}



void increment_tree()
{
    leafHarai = 0;
    for(int i=node_to_delete.size()-1; i>=0; i--)
    {
        Incremental_DB *temp = node_to_delete[i];
        if(temp->condition == 1)
        {
            if(temp->split_node == NULL)
                continue; //not valid
            if(temp->split_node->parent == NULL)
                continue; //not valid

            leafHarai++; //logically one leaf gets lost
            //basically direct '$' connection******
            temp->split_node->parent->child[text[temp->activeEdge]-BASE] = NULL; //edge deleted
            int cnt=0;
            int save;
            for(int j=0; j<MAX_CHAR; j++)
            {
                if(temp->split_node->parent->child[j] != NULL)
                {
                    cnt++;
                    if(cnt == 1)
                    {
                        save = j;
                    }
                }
                if(cnt>1)
                    break;
            }
            if(cnt==0)
            {
                //its a leaf
                if(temp->split_node->parent != root)
                {
                    //develop leaf end
                    int length = edgeLength(temp->split_node->parent);
                    temp->split_node->parent->end = &leafEnd;
                    temp->split_node->parent->start = leafEnd-length+1;

                    if(temp->split_node->parent->reverseSuffixLink != NULL)
                    {
                        //this has become a leaf node now
                        //so it can't be an suffixLink Node
                        temp->split_node->parent->reverseSuffixLink->suffixLink = root;
                    }
                    temp->split_node->parent->suffixLink = root;  //this is already a leaf Node

                    //a leaf Node created
                    leafHarai--;
                }
            }
            else if(cnt == 1)
            {
                //need to merge
                if(temp->split_node->parent != root)
                {

                    int length1 = edgeLength(temp->split_node->parent);
                    int length2 = edgeLength(temp->split_node->parent->child[save]);

                    int ch = temp->split_node->parent->start;
                    //edge correction
                    temp->split_node->parent->parent->child[text[ch]-BASE] = temp->split_node->parent->child[save];
                    //parent correction ? ?
                    temp->split_node->parent->child[save]->parent = temp->split_node->parent->parent;
                    //start correction
                    temp->split_node->parent->child[save]->start =  *(temp->split_node->parent->child[save]->end) - (length1+length2)+1;

                    //suffixLink correction
                    if(temp->split_node->parent->reverseSuffixLink != NULL)
                    {
                        //as this node doesn't exist, this can't be considered as suffix Link any more
                        temp->split_node->parent->reverseSuffixLink->suffixLink = root;
                    }
                    //node deletion
                    temp->split_node->parent->parent = NULL; //deleted
                }

            }
            else
            {
                //nothing to merge this is not leaf
            }
            //delete split_node
            temp->split_node->parent = NULL;
        }
        else
        {
            //condition 2
            //split nodes
            if(temp->split_node == NULL || temp->split_node->parent == NULL)
                continue; //invalid node
            leafHarai++;

            if(temp->successor->parent == NULL)
            {
                //debug: cout<<"succesor nai "<<endl;
                //successor edge is deleted
                temp->split_node->child['$'-BASE] = NULL; //'$' edge deleted
                //need to check if it has become an end node or not
                int cnt=0;
                int save;
                for(int j=0; j<MAX_CHAR; j++)
                {
                    if(temp->split_node->child[j] != NULL)
                    {
                        cnt++;
                        if(cnt == 1)
                        {
                            save = j;
                        }
                    }
                    if(cnt>1)
                    {
                        break;
                    }
                }
                if(cnt == 0)
                {
                    //debug: cout<<"dhuke***"<<endl;
                    //it has become a leaf node
                    if(temp->split_node != root)
                    {
                        int length = edgeLength(temp->split_node);
                        //debug: cout<<"length = " << length<<" "<<leafEnd<<" "<<temp->split_node->start<<" "<<*(temp->split_node->end)<<endl;
                        temp->split_node->end = &leafEnd;
                        temp->split_node->start = *(temp->split_node->end)-length+1;
                        //debug: cout<<"length = " << length<<" "<<leafEnd<<" "<<temp->split_node->start<<" "<<*(temp->split_node->end)<<endl;
                        if(temp->split_node->reverseSuffixLink != NULL)
                        {
                            temp->split_node->reverseSuffixLink->suffixLink=root;
                        }
                        temp->split_node->suffixLink = root; //its now a leaf node
                        leafHarai--;
                    }
                }
                else if(cnt == 1)
                {
                    //need to merge
                    if(temp->split_node != root)
                    {
                        //edge creation
                        int ch = temp->split_node->start;
                        int length1 = edgeLength(temp->split_node);
                        int length2 = edgeLength(temp->split_node->child[save]);
                        //start correction
                        temp->split_node->child[save]->start = *(temp->split_node->child[save]->end) - (length1+length2)+1;
                        //parent correction
                        temp->split_node->child[save]->parent=temp->split_node->parent;
                        //edge creation
                        temp->split_node->parent->child[text[ch]-BASE] = temp->split_node->child[save];

                        //suffix Link correction
                        if(temp->split_node->reverseSuffixLink != NULL)
                        {
                            temp->split_node->reverseSuffixLink->suffixLink=root;
                        }

                        //delete node
                        temp->split_node->parent = NULL;
                    }
                }
                else
                {
                    //no need to merge
                    //no need to change end of this node
                }

            }
            //everythings normal
            else if(temp->successor->parent != NULL)
            {
                //debug: cout<<"succesor ache "<<endl;
                if(temp->split_node != root)
                {
                    int length1 = edgeLength(temp->split_node);
                    int length2 = edgeLength(temp->successor);
                    int ch = temp->split_node->start;
                    //start correction
                    temp->successor->start = *(temp->successor->end) - (length1+length2) + 1;
                    //debug: cout<<"start hoi = " << temp->successor->start<<" "<<*(temp->successor->end)<<endl;
                    //parent correction
                    temp->successor->parent = temp->split_node->parent;
                    //edge creation
                    temp->split_node->parent->child[text[ch]-BASE] = temp->successor;
                    //suffix Link correction
                    if(temp->split_node->reverseSuffixLink != NULL)
                    {
                        temp->split_node->reverseSuffixLink->suffixLink=root;
                    }
                    //node delete
                    temp->split_node->parent=NULL;
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

void printAllSuffix(SuffixTreeNode *node,vector<char>V)
{


    bool okay = false;
    for(int i=0; i<=MAX_CHAR; i++)
    {
        if(node->child[i] != NULL)
        {
            okay=true;
            char ch=i;
            //cout<<"i = " << i << " "<<ch<<endl;
            //cout<<"start = " << node->child[i]->start<<" "<< "end = " << *(node->child[i]->end)<<" "<<endl;
            string s="";
            for(int j=node->child[i]->start; j<=*(node->child[i]->end); j++)
            {
                s=s+text[j];
                V.push_back(text[j]);
            }
            printAllSuffix(node->child[i],V);
            for(int j=node->child[i]->start; j<=*(node->child[i]->end); j++)
            {
                V.pop_back();
            }
        }
    }
    if(!okay)
    {
        //cout<<"pass complete" <<endl;
        for(int i=0; i<(int)V.size(); i++)
        {
            cout<<V[i];
        }
        cout<<endl;
    }
    return;
}



//function to build suffix tree
void buildSuffixTree() {
    rootEnd = new int();
    *rootEnd = -1;
    root = newNode(-1,rootEnd);
    activeNode = root;
    for(int i=0; i<(int)text.size(); i++) {
        extendSuffixTree(i);
    }
}


void decrement_tree(int pos)
{
    SuffixTreeNode *temp = root;
    //identify the branch
    while(true)
    {
        SuffixTreeNode *notun = temp->child[text[pos]-BASE];
        int new_pos = pos+edgeLength(temp->child[text[pos]-BASE]);
        //cout<<"new_pos " << new_pos<<" "<<pos<<endl;

        if(new_pos==(int)text.size())
        {
            //this leaf node needs to be deleted
            temp=notun;
            break;
        }
        temp=notun;
        pos=new_pos;
    }

    int len = edgeLength(temp);
    if(len == 1 && text[temp->start - BASE] == '$')
    {
        //$ edge
        if(temp->parent == root)
        {
            //should handle
            //just '$' suffix
        }
        else
        {
            //basically ab-$ means I need to delete ab
            temp->parent->child['$']=NULL; //deleting edge
            SuffixTreeNode *intermediate = temp;
            temp=temp->parent; //basically need to delete parent
            intermediate->parent=NULL; //deleting parent
        }
    }

    //debug: cout<<"done "<<temp->start<<" "<<*(temp->end)<<endl;
    //delete branch and modify stuffs
    temp->parent->child[text[temp->start]-BASE]=NULL; //edge delete

    //modify node
    bool $_exist=false;
    int cnt = 0;
    int save;
    for(int i=0; i<MAX_CHAR; i++)
    {
        if(temp->parent->child[i] != NULL)
        {
            cnt++;
            if(i == '$')
            {
                $_exist=true;
                break;
            }
            if(cnt == 1)
            {
                save = i;
            }
        }
    }
    if($_exist)
    {
        //nothing to do
    }
    else
    {
        if(cnt==0)
        {
            //needs to add the leafEnd
            if(temp->parent!= root)
            {
                int length = edgeLength(temp->parent);
                //end correction
                temp->parent->end= &leafEnd;
                //start correction
                temp->parent->start = leafEnd - length + 1;
                //suffix Link correction
                if(temp->parent->reverseSuffixLink != NULL)
                {
                    //this has become a leaf
                    temp->parent->reverseSuffixLink->suffixLink = root;
                }

            }

        }
        else if(cnt == 1)
        {
            //only one branch and without $
            if(temp->parent != root)
            {
                int length1 = edgeLength(temp->parent);
                int length2 = edgeLength(temp->parent->child[save]);
                int ch = temp->parent->start;

                //modify start
                temp->parent->child[save]->start = *(temp->parent->child[save]->end) - (length1+length2) + 1;
                //modify parent
                temp->parent->child[save]->parent=temp->parent->parent;
                //edge creation
                temp->parent->parent->child[text[ch]-BASE] = temp->parent->child[save];
                //suffix Link modification
                if(temp->parent->reverseSuffixLink != NULL)
                {
                    //this node doesn't belong so suffix Link can't point there
                    temp->parent->reverseSuffixLink->suffixLink = root;
                }
                temp->parent->parent=NULL; //parent lost node lost
            }
        }
        else
        {
            //more than one branch
            //this node is fine
        }
    }
    temp->parent=NULL; //node delete
    if(temp->reverseSuffixLink != NULL) {
		//leaf node's suffixlink will be root 
		temp->reverseSuffixLink->suffixLink=root; 
	}
}




void initialize_for_new_insertion(int got)
{
    if(got == -1)
    {
        //all suffixes are present
        //move from root
        activeNode = root;
        activeLength = 0;
        activeEdge = -1;
        remainingSuffixCount = 0;
    }
    else
    {
        int pos=got;
        remainingSuffixCount = text.size()-2 - got+1;
        SuffixTreeNode *temp = root;
        while(true)
        {
            activeNode = temp;
            activeLength = 0;
            activeEdge = pos;
            int fix=pos;
            bool ok=false;
            if(temp->child[text[fix]-BASE] == NULL)
                return;
            //cout<<temp->child[text[fix]-BASE]->start<<endl;
            for(int i=temp->child[text[fix]-BASE]->start; i<=*(temp->child[text[fix]-BASE]->end); i++)
            {
                //cout<<text[i]<<endl;
                if(pos<=((int)text.size()-2) && text[i] == text[pos])
                {
                    pos++;
                    activeLength++;
                    ok=true;
                }
                else
                {
                    ok=false;
                    return;
                }
            }
            if(ok)
            {
                temp=temp->child[text[fix]-BASE];
            }

        }
    }
}

void clear_occurence_vector(SuffixTreeNode *node)
{
    if(node == NULL)
        return;
    for(int i=0; i<MAX_CHAR; i++)
    {
        if(node->child[i] != NULL)
        {
            clear_occurence_vector(node->child[i]);
        }
    }
    node->occurence_vector.clear();
    return;
}

deque<int> make_occurence_vector(SuffixTreeNode *node, int depth)
{
    bool move = false;
    for(int i=0; i<MAX_CHAR; i++)
    {
        if(node->child[i] != NULL)
        {
            move=true;
            deque<int>temp;
            temp = make_occurence_vector(node->child[i],depth+edgeLength(node->child[i]));
            for(int j=0; j<(int)temp.size(); j++)
            {
                node->occurence_vector.push_back(temp[j]);
            }
        }
    }
    sort(node->occurence_vector.begin(),node->occurence_vector.end());
    if(move)
        return node->occurence_vector;
    else
    {
        node->occurence_vector.push_back(text.size()-depth);
        sort(node->occurence_vector.begin(),node->occurence_vector.end());
        return node->occurence_vector;
    }
}

void print_occurence_vector(SuffixTreeNode *node)
{

    for(int i=0; i<MAX_CHAR; i++)
    {
        if(node->child[i] != NULL)
        {
            string s="";
            for(int j=node->child[i]->start; j<=*(node->child[i]->end); j++)
            {
                s=s+text[j];
            }
            cout<<s<<endl;
            for(int j=0; j<(int)node->child[i]->occurence_vector.size(); j++)
            {
                cout<<node->child[i]->occurence_vector[j]<<" ";
            }
            cout<<endl;
            print_occurence_vector(node->child[i]);
        }
    }
    return;
}

void general_suffix_tree()
{
    fp = fopen("general_input.txt","r");
    char arr[100000];
    fscanf(fp,"%s",arr);
    text="";
    for(int i=0; i<(int)strlen(arr); i++)
    {
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



//menu option
void begin_code(){
    cerr <<"initial string = " << endl;
    cin>>text;

    //initial suffix tree
    initialize();
    buildSuffixTree();

    string new_input;
    vector<char>V;
    
    int deletion_ptr=-1; //to indicate upto which index is deleted
    
    while(true){
        cerr <<"Press 0 for exit"<<endl;
        cerr <<"Press 1 for new increment"<<endl;
        cerr <<"Press 2 for decrement"<<endl;
        cerr <<"Press 3 for mine"<<endl;

        int choice,length;
        cin>>choice;
        if(choice==1){
            cerr <<"New string to append: "<<endl;
            cin>>new_input;

            //modification
            leafEnd--;
            increment_tree();
            leafHarai--;

			//need to find the activeNode,activeEdge,activeLength,remainingSuffixCnt
			int sp = text.size()-2 - leafHarai + 1;
			if(leafHarai == 0) sp= -1;
			initialize_for_new_insertion(sp);
			remainingSuffixCount = leafHarai;
			
            length = text.size()-1;
            text[length]=new_input[0];
            for(int i=1;i<(int)new_input.size();i++){
                text=text+new_input[i];
            }

            text=text+'$';

            for(int i=length;i<(int)text.size();i++){
                extendSuffixTree(i);
            }

        }
        else if(choice==2){
			//deletion
			cerr <<" deleted upto "<<deletion_ptr << "th index " << " max deletion possible upto " << text.size()-2<<" th index " << endl;
			int number; //number to characters to delete
			cerr <<"How many characters to delete ";
			cin>>number;
			
			for(int i=1;i<=number;i++){
				deletion_ptr++;
				decrement_tree(deletion_ptr);
			}
        }
        else if(choice == 0){
            return;
        }
        else if(choice == 3){
            V.clear();
            printAllSuffix(root,V);
        }

    }
}



int main()
{

    //freopen("in1.txt","r",stdin);
    //freopen("out1.txt","w",stdout);

    double st = clock();

    srand(time(NULL));

    begin_code();

    double ed = clock();

    cerr << (ed-st)/CLOCKS_PER_SEC << endl;

    return 0;

}

