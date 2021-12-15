# Contributing

Here you find some guidelines for your contributions.

## Getting Started
Make sure you have a GitHub account

### Opening an issue
Before creating a new issue please look for similar issues already exists:
  * in case you found it
    * read it carefully
    * add your comments to the existing issue
	* add comment to an issue only if related to it (otherwise choose a different issue or create a new one).
  * otherwise create a new issue on the GitHub repository
     * Clearly describe the issue
	 * do not write long text in issue title, choose a short title for the topic, then add your text to the issue body
     * if it's a bug include also steps to reproduce it
     * Make sure you fill in the earliest version that you know has
   the issue

If you just want to notify bugs or suggesting improvements you can just open an issue on GitHub, otherwise
[Fork] the repository on GitHub, then make your changes and create a pull request.

### Making Changes
Follow these steps, paying attention to adhere to the rules described in the next paragraphs.
* [Fork](http://help.github.com/fork-a-repo/) the project, clone your fork, and configure the remotes.
* Create a topic branch from where you want to base your work
  * This is usually the master branch
  * Only target release branches if you are certain your fix must be
   on that branch
  * Name branches with the type of issue you are fixing;
   `fix`, `feat`, `chore`, `docs` (e.g. `fix/my_bug_fix`)
  * Please avoid working directly on the master branch
* Make commits of logically related units (e.g. do not mix unrelated changes)
* Make sure your commit messages are in the proper format
* Push your topic branch up to your fork.
* [Open a Pull Request] with a clear title and description.

## Coding Conventions
We tried to follow a common coding style,
anyway if you find some not compliant code, you are encouraged to change
it (this should be classified as a `refactor` change).

To ensure all code is consistent and readable, we adhere to the
default coding conventions utilized in Visual Studio. The easiest
first step is to auto format the code within Visual Studio (if possible)
with the key combination of `Ctrl + k` then `Ctrl + d` which will ensure
the code is correctly formatted and indented.


## Documentation

All header files that require documentation need to contain
inline code documentation suitable for [Doxygen], a program that parses
the sources and generates the documentation. 

## Commit Messages

The commit message lines should never exceed 72 characters and should
be entered in the following format:

~~~
type(scope): subject
(blank line)
body
(new line at the end)
~~~
See below for details.

### Type

The type must be one of the following:
  * feat: A new feature
  * fix: A bug fix
  * docs: Documentation only changes
  * refactor: A code change that neither fixes a bug or adds a feature
  * perf: A code change that improves performance
  * test: Adding missing tests
  * chore: Changes to the build process or auxiliary tools or libraries
  
### Scope

The scope could be anything specifying the place of the commit change.

### Subject

The subject contains succinct description of the change.

### Body

The body should include the motivation for the change,
compared with the previous behavior.
References to previous commit hashes is actively encouraged if they are relevant.

**Example** 

~~~
doc(CONTRIBUTING): example of commit message 
  
An example of commit message was added to the Commit Messages section.
  
After the example a brief explanation of all message elements was added
to explain their meaning and usage.
~~~

Here the type is `doc`, the scope is `CONTRIBUTING`, the subject is `example of commit message` and the remainder is the body (a brief description followed by motivations, remarks, etc.).


## Submitting Changes
  * Push your changes to your topic branch in your repository.
  * Submit a pull request to this repository.
  * We will look at the pull request and provide feedback where required.
  * The waiting time for a feedback depends on our availability and the complexity of the issue



[Open a Pull Request]: https://help.github.com/articles/using-pull-requests/
[Doxygen]: http://www.doxygen.org/index.html

