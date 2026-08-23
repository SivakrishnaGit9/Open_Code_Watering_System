---
name: domain-modeling
description: Build and sharpen a project's domain model. Use when discussing codebase terminology, writing or editing a CONTEXT.md, or recording or editing an ADR.
---

# Domain Modeling

Actively build and sharpen the project's domain model as you design. This is the *active* discipline: challenging terms, inventing edge-case scenarios, and writing the glossary and decisions down the moment they crystallise.

## File structure

Most repos have a single context:

```
/
├── CONTEXT.md
├── docs/
│   └── adr/
│       ├── 0001-event-sourced-orders.md
│       └── 0002-postgres-for-write-model.md
└── src/
```

Create files lazily: only when you have something to write. If no `CONTEXT.md` exists, create one when the first term is resolved. If no `docs/adr/` exists, create it when the first ADR is needed.

## During the session

### Challenge against the glossary
When the user uses a term that conflicts with the existing language in `CONTEXT.md`, call it out immediately.

### Sharpen fuzzy language
When the user uses vague or overloaded terms, propose a precise canonical term.

### Discuss concrete scenarios
When domain relationships are being discussed, stress-test them with specific scenarios.

### Cross-reference with code
When the user states how something works, check whether the code agrees.

### Update CONTEXT.md inline
When a term is resolved, update `CONTEXT.md` right there. Use the format in [CONTEXT-FORMAT.md](./CONTEXT-FORMAT.md).

### Offer ADRs sparingly
Only offer to create an ADR when all three are true:
1. **Hard to reverse**
2. **Surprising without context**
3. **The result of a real trade-off**
Use the format in [ADR-FORMAT.md](./ADR-FORMAT.md).
