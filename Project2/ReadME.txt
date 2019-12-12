Project 2
Nathan Redmond and Lucas Martin
Last Updated: 9 December 2019

About:
  This project was a goal to build a miniature google search. Our professor gave us two files, Metadata.txt and 
Citations.txt. In the metadata file there were list of articles with a page id, title, author, and abstract of what 
the paper was about all separated by "+++++". The citations file was a list of page ids and what articles that page id references separated by "+++++" and "-----" between page id and references. 

Our goal and attempt:
  We created two programs, one to split up the metadata file in parallel and another one to search a word with feedback.
To split up the metadata we have each node start reading the file in blocksizes. Each node will parse the abstract paragraph
and insert each word into a binary search tree and also insert the page id in the linked list attached to the tree node.
However, the file is so large our BST fills up and runs out of memory, so everytime the tree fills up we print the data
of the tree to a file called backward_index.txt and then delete the tree and keep running. This will give many duplicates
of words in the backward_index file but we take care of this in the other program. The second program we ask for a search
word or sentence and then read from the file backward_index.txt to create another tree with the words you searched. From 
here we can use the words found along with the page ids linked to them and create a adjacency matrix of page ids linking
to eachother. We then find the eigenvector using the adjacency matrix to determine which page id is most relevent out of
the given page ids. So final result is after searching a word or sentence is returning the top 10 page ids related to
your search.
