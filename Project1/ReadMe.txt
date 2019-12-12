*****************************
Nathan Redmond
Lucas Martin
Project 1- Password Cracking
*****************************

About:
  Attempting a password chracking program in MPI parallel. Read from a shadow file to parse up username and encrypted
password. Read dictionary file in parallel to encrypt word and check against encrypted password. 

Attmept and Approach:
  Starting off we made a function to open up the shadow file and parse it into separate arrays. This way we can have
the username and encrypted password and algorithm used for the encryption into alloced arrays. After we parsed the 
shadow file. We ran a while loop for each node to grab a word in the dictionary file to ecrypt it and compare against 
shadow file encrypted password. We also check up to four digits attached to the front and back of a dictionary word.
We ran the function fork() when checking the digits in front or back, becasue the child process will check the digits
in the front and parent process will check the digits attached to the back of a word. 

Given the size of the dictionary we had and how many digits we tested front and back. Our program found all of the 
possible passwords that was given from our professor in under 15 minutes! See output.log
