# P2P_File_Sharing

# Aim:
Build a group based file sharing system where users can share, download files from the group they belong to. Download should be parallel with multiple pieces from multiple peers.

# Prerequisites
Socket Programming, SHA1 hash, Multi-threading

# Architecture Overview:
The Following entities will be present in the network :
# 1. Synchronized trackers(2 tracker system):
a. Maintain information of clients with their files(shared by client) to assist the clients for the communication between peers.\
b. Trackers should be synchronized i.e all the trackers if online should be in sync with each other.

# 2. Clients:
a. User should create an account and register with tracker.\
b. Login using the user credentials.\
c. Create Group and hence will become owner of that group.\
d. Fetch list of all Groups in server.\
e. Request to Join Group.\
f. Leave Group.\
g. Accept Group join requests (if owner)h. Share file across group: Share the filename and SHA1 hash of the complete file as well as piecewise SHA1 with the tracker.\
i. Fetch list of all sharable files in a Group.\
j. Download file.\
k. Show downloads
l. Stop sharing file.\
m. Stop sharing all files(Logout).\
n. Whenever client logins, all previously shared files before logout should automatically be on sharing mode.
