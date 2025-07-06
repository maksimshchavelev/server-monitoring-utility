


### Guide for Contributions:

#### General Commit Formatting Rules:

-   The commit header and body must be informative, concise, and written in English. Use imperative mood verbs in header.
-   Avoid using articles in header.
-   The commit header should be limited to **90-100 characters**, and each line of the commit body should also be limited to **90-100 characters**.

----------

#### Commit Header Formatting Rules:

**Format:** `type, module: short description`

 -   **type**: The type of commit (see below):
 -   **module**: The module being modified in program. You can ignore the module if the changes are not in the module.

**There are the following types of commits:**

 - **feat**: Adding a new feature or functionality. 
 - **fix**: Fixing bugs or errors in the code. 
 - **perf**: Changes that do not add new functionality or fix bugs, but rather make the code more efficient or improve performance (for example, reducing memory usage or optimizing).
 - **docs**: Updating or improving documentation (e.g., README, in-code comments). 
 - **style**: Code style changes (formatting, white-space, missing semi-colons, etc.). 
 - **refactor**: Code refactoring without changing its external behavior. 
 - **test**: Adding or modifying tests. 
 - **chore**: Maintenance tasks not affecting code logic (e.g., dependency updates, build scripts).

----------

#### Documentation Style:

Documentation should follow the **Doxygen** style (more details in [CODE_STYLE.md](./CODE_STYLE.md)).
