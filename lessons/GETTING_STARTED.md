# Getting Started with SIMPL Lessons

Welcome to the SIMPL learning journey! This guide will help you fork the repository and start working through the lessons.

## Prerequisites

Before you begin, make sure you have:

- **Windows OS** with Visual Studio 2019 or later (with C++ development tools)
- **Git** installed on your system
- **GitHub account** (free account is fine)
- Basic familiarity with C++ and command line

## Step 1: Fork the Repository

1. **Navigate to the SIMPL repository:**
   - Go to [https://github.com/mvpete/simpl](https://github.com/mvpete/simpl)

2. **Click the "Fork" button:**
   - Look for the "Fork" button in the top-right corner of the page
   - Click it to create your own copy of the repository

3. **Wait for the fork to complete:**
   - GitHub will create a copy under your account: `https://github.com/YOUR-USERNAME/simpl`

## Step 2: Clone Your Fork

1. **Open your terminal or command prompt**

2. **Clone your forked repository:**
   ```bash
   git clone https://github.com/YOUR-USERNAME/simpl.git
   cd simpl
   ```

3. **Add the original repository as upstream** (to get updates):
   ```bash
   git remote add upstream https://github.com/mvpete/simpl.git
   ```

4. **Verify your remotes:**
   ```bash
   git remote -v
   ```
   
   You should see:
   ```
   origin    https://github.com/YOUR-USERNAME/simpl.git (fetch)
   origin    https://github.com/YOUR-USERNAME/simpl.git (push)
   upstream  https://github.com/mvpete/simpl.git (fetch)
   upstream  https://github.com/mvpete/simpl.git (push)
   ```

## Step 3: Set Up Your Development Environment

1. **Open the solution in Visual Studio:**
   - Navigate to the cloned repository folder
   - Double-click `simpl.sln` to open in Visual Studio

2. **Build the solution:**
   - Right-click on the solution in Solution Explorer
   - Select "Build Solution" (or press Ctrl+Shift+B)
   - Ensure there are no build errors

3. **Run the tests (optional but recommended):**
   - Right-click on `simpl.test` project
   - Select "Build"
   - Open Test Explorer: Test → Test Explorer
   - Click "Run All" to verify everything works

## Step 4: Start Learning!

### Lesson Navigation

The lessons are located in the `lessons/` directory:

- **L0.md** - Getting Started (Start here!)
- **L1.md** - Tokenizer
- **L2.md** - Parser
- **L3.md** - Engine & Evaluation
- **L4.md** - Adding Features
- **L5.md** - Objects and Inheritance
- **L6.md** - Multiple Dispatch
- **L7.md** - Arrays and Collections
- **L8.md** - Functions and Closures
- **L9.md** - Working with Libraries
- **L10.md** - Building Complete Applications
- **L11.md** - Contributing to SIMPL

### Recommended Learning Path

1. **Read the lesson:** Open `lessons/L0.md` in your favorite markdown reader or GitHub
2. **Follow the instructions:** Each lesson has explanations and examples
3. **Complete the task:** Each lesson includes a hands-on contribution task
4. **Test your code:** Run your SIMPL programs or tests
5. **Commit your work:** Save your progress with git
6. **Move to next lesson:** Click the "Next" link at the bottom of each lesson

### Example: Starting Lesson 0

1. **Read the lesson:**
   ```bash
   # View in terminal (if you have a markdown viewer)
   # Or open lessons/L0.md in VS Code, notepad, or browser
   ```

2. **Complete the task:**
   - Create `examples/greeting.sl` as instructed
   - Test it with the REPL

3. **Commit your work:**
   ```bash
   git add examples/greeting.sl
   git commit -m "Add personal greeting example"
   ```

## Step 5: Working with Git

### Basic Workflow

After completing each lesson's task:

```bash
# Check what files you've changed
git status

# Add your new files or changes
git add <filename>

# Commit with a descriptive message
git commit -m "Descriptive message about what you did"

# Push to your fork (optional)
git push origin main
```

### Creating a Branch for Your Work (Recommended)

It's good practice to work in a branch:

```bash
# Create and switch to a new branch
git checkout -b my-simpl-lessons

# After making changes and commits
git push origin my-simpl-lessons
```

### Syncing with Upstream (Getting Updates)

If the original repository gets updates:

```bash
# Fetch updates from upstream
git fetch upstream

# Merge updates into your branch
git merge upstream/main

# Or rebase your changes on top
git rebase upstream/main
```

## Step 6: Running SIMPL Programs

### Using the REPL (Interactive Mode)

1. **Set simpl.repl as startup project:**
   - Right-click `simpl.repl` in Solution Explorer
   - Select "Set as StartUp Project"

2. **Run the REPL:**
   - Press F5 or click "Start"
   - Type SIMPL commands interactively

3. **Example session:**
   ```
   > @import io
   > println("Hello from SIMPL!");
   Hello from SIMPL!
   > let x = 10 + 5;
   > println(x);
   15
   ```

### Running SIMPL Files

1. **Configure command arguments:**
   - Right-click `simpl.repl` → Properties
   - Go to Debugging → Command Arguments
   - Set to: `..\examples\your_file.sl`

2. **Run the file:**
   - Press F5
   - Your program executes and shows output

### Quick Test

Create a simple test file `examples/test.sl`:
```simpl
@import io
println("SIMPL is working!");
```

Run it to verify everything is set up correctly.

## Tips for Success

### Learning Tips

✅ **Take your time** - Don't rush through the lessons  
✅ **Experiment freely** - Try variations of the examples  
✅ **Debug actively** - Use print statements to understand what's happening  
✅ **Ask questions** - Open GitHub issues if you're stuck  
✅ **Share progress** - Commit your work regularly  

### Common Issues

**Build Errors:**
- Make sure you have C++ development tools installed in Visual Studio
- Try "Clean Solution" then "Rebuild Solution"

**REPL Not Starting:**
- Check that simpl.repl is set as startup project
- Verify the build was successful

**Git Issues:**
- Make sure you cloned YOUR fork, not the original repo
- Check remote URLs with `git remote -v`

**Test Failures:**
- Some tests might fail initially - that's expected!
- Focus on the tests related to your current lesson

### Getting Help

- **GitHub Issues:** [https://github.com/mvpete/simpl/issues](https://github.com/mvpete/simpl/issues)
- **Original Repo:** Check the main README for more info
- **Lessons Directory:** Read the lesson files for detailed instructions

## Progress Tracking

Create a checklist to track your progress:

- [ ] Forked and cloned repository
- [ ] Built solution successfully
- [ ] Ran REPL successfully
- [ ] Completed Lesson 0 - Getting Started
- [ ] Completed Lesson 1 - Tokenizer
- [ ] Completed Lesson 2 - Parser
- [ ] Completed Lesson 3 - Engine & Evaluation
- [ ] Completed Lesson 4 - Adding Features
- [ ] Completed Lesson 5 - Objects and Inheritance
- [ ] Completed Lesson 6 - Multiple Dispatch
- [ ] Completed Lesson 7 - Arrays and Collections
- [ ] Completed Lesson 8 - Functions and Closures
- [ ] Completed Lesson 9 - Working with Libraries
- [ ] Completed Lesson 10 - Building Complete Applications
- [ ] Completed Lesson 11 - Contributing to SIMPL

## Next Steps

Once you're set up:

1. **Open `lessons/L0.md`** and start reading
2. **Follow the instructions** step by step
3. **Complete the contribution task** for each lesson
4. **Commit your work** after each lesson
5. **Move to the next lesson** using the "Next" links

## Contributing Back (Optional)

If you create something cool or find improvements:

1. **Push your changes** to your fork
2. **Create a Pull Request** to the original repository
3. **Share your learnings** with the community

---

**Ready to start?** Open `lessons/L0.md` and begin your SIMPL journey! 🚀

**Questions?** Open an issue on GitHub or check the lesson files for detailed guidance.

Happy coding!
