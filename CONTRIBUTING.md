# Contributing Guidelines

Thanks for contributing to this project! Please follow these simple guidelines to keep our workflow clean and consistent.

## 1. Getting Started
- Fork the repository and clone your fork:
  ```bash
  git clone https://github.com/<your-username>/<repo-name>.git
  ```
- Create a new branch for your work:
  ```bash
  git checkout -b feature/<short-description>
  ```

## 2. Code Style
- Keep functions short and focused.
- Write clear, descriptive variable and function names.
- Add comments for non-obvious logic.

## 3. Commits
- Write clear, concise commit messages:
  ```
  <type>: <short summary>

  Example:
  fix: handle null pointer in read_config()
  feat: add CLI argument parsing
  ```
  Common types: `feat`, `fix`, `refactor`, `docs`, `test`, `chore`

## 4. Pull Requests
<!-- - Ensure your code compiles with `make` and passes any existing tests. -->
- Ensure your code compiles and passes any existing tests.
- Keep PRs focused — one feature/fix per PR.
- Describe what your change does in the PR description.

## 5. Testing
- Add or update tests for any new functionality.
<!-- - Run all tests before submitting:
  ```bash
  make test
  ``` -->

## 6. Communication
- Use GitHub Issues for bugs or suggestions.
- Be respectful and constructive in discussions.
