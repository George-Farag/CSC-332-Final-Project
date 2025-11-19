# Shared Memory and Pipe Communication

## What This Does
This program shows how two processes can communicate super fast using:
- **Shared Memory** = Like a whiteboard both can write on and read from
- **Pipe** = Like a doorbell to signal "data is ready!"

## The Simple Explanation

**Parent Process:**
1. Writes message on the whiteboard (shared memory)
2. Rings the doorbell (sends signal through pipe)
3. Waits for child to finish

**Child Process:**
1. Waits for doorbell to ring (reads from pipe)
2. Reads message from whiteboard (shared memory)
3. Prints the message

**Result:** Fast data transfer with perfect timing!

## How to Run

### Step 1: Open WSL (Windows Subsystem for Linux)
```bash
wsl
```

### Step 2: Go to project folder
```bash
cd /mnt/c/Users/elasm/CSC-332-Final-Project
```

### Step 3: Compile
```bash
g++ -std=c++11 sharedmempipe.cpp -o sharedmempipe -lrt -pthread
```

### Step 4: Run
```bash
./sharedmempipe
```

### You'll see:
```
Child: read from shared memory: "Hello from parent using shared memory!"
```

## Why Both Shared Memory AND Pipes?

**Shared Memory (Whiteboard):**
- ✅ Super fast - no copying data
- ❌ Needs coordination

**Pipe (Doorbell):**
- ✅ Perfect for signaling
- ✅ Automatic synchronization

**Together = Fast + Safe!**

## What Each Part Does

- `shm_open()` - Create the whiteboard
- `mmap()` - Let process access the whiteboard
- `pipe()` - Create the doorbell
- `fork()` - Split into parent and child
- `write()` - Ring the doorbell
- `read()` - Wait for doorbell to ring
