## Module Data Transmission Protocol
In this chapter, we will discuss the **Module Data Transmission Protocol** (hereinafter **MDTP**) — a binary format for transferring data from a module to the server core, which was adopted in connection with the introduction of dynamically loaded modules for convenience. In addition, this method of data transfer is more efficient and less voluminous than **Json**.

> There will be no code in this guide. Consider it part of the guide to creating your own SDK (to be described in the next chapter).



## MDTP concept
MDTP is a recursive protocol. The basic concept is that each node is either a **value** or a **container**. Each node stores the own type and child nodes (children can be either containers or values), allowing us to recursively descend lower and lower until we reach a value (a "leaf" in the context of trees)



## Internal structure of MDTP
The MDTP device is quite simple. Let's go through it step by step.
First comes the header:

```
[version of MDTP]: 1 unsigned byte
[payload size]: unsigned int32
----------- end of header -----------
[payload...]
```

> All data is encoded in [Big Endian](https://en.wikipedia.org/wiki/Endianness).

- MDTP version - this is the MDTP version. **Currently, the MDTP version is `1`**
- Payload size - the size of the rest of the message. For example, if the message contains only one child with a size of 42 bytes, then the value here will be 42
- Payload is payload

Let us continue. Let's examine the structure of a container node.

```
[node type]: 1 unsigned byte (0 because node is container)
[node name length]: unsigned int32
[name of node...]: array of char
[payload size]: unsigned int32
[payload...]
```

- Node type: node type, can take the following values: `0` - if the node is a **container** and `1` - if the node is a **value**
- Node name length - length of the node name **excluding the terminating zero** (‘\0’).
- Name of node - name of the node **without terminating zero**
- Payload size - size of nested nodes.
- Payload - the nested nodes themselves


Now let's consider the value node:

```
[node type]: 1 unsigned byte (1 because node is value)
[node name length]: unsigned int32
[name of node...]: array of char
[units length]: unsigned int32
[units...]: array of char
[value length]: unsigned int32
[value...]: array of char
```

- Node type: node type (`1` because type of node is **value**)
- Node name length - length of the node name **excluding the terminating zero** (`\0`).
- Name of node - name of the node
- Units length - length of string with units of measurement **without terminating zero**
- Units - string with units of measurement **without terminating zero**
- Value length - length of string with value  **without terminating zero**
- Value - String with the value **without terminating zero**



## Examples

### Example 1: A simple value node

Imagine a module wants to report its version number to the server.

```
Header:
[version]                 : 0x01 (1)
[payload size]            : 0x0000001A (26 bytes)

Payload (value node):
[node type]               : 0x01 (value)
[node name length]        : 0x00000007 (7 bytes)
[node name]               : "version"
[units length]            : 0x00000000 (0 bytes)
[value length]            : 0x00000006 (6 bytes)
[value]                   : "1.0.42"
```

Explanation:

- The node is a value (1).
- The node name is "version".
- There are no units.
- The value string is "1.0.42".
- This way, the server knows the module version without parsing any JSON.



### Example 2: Container with nested nodes

Now let’s say a module reports CPU metrics. We use a container node called "cpu", which contains two child values: "temperature" and "frequency".

```
Header:
[version]                 : 0x01 (1)
[payload size]            : 0x00000043 (67 bytes)

Payload (container "cpu"):
[node type]               : 0x00 (container)
[node name length]        : 0x00000003 (3 bytes)
[node name]               : "cpu"
[payload size]            : 0x00000037 (55 bytes)

  Child #1 (value "temperature"):
  [node type]             : 0x01 (value)
  [node name length]      : 0x0000000B (11 bytes)
  [node name]             : "temperature"
  [units length]          : 0x00000001 (1 byte)
  [units]                 : "C"
  [value length]          : 0x00000002 (2 bytes)
  [value]                 : "75"

  Child #2 (value "frequency"):
  [node type]             : 0x01 (value)
  [node name length]      : 0x00000009 (9 bytes)
  [node name]             : "frequency"
  [units length]          : 0x00000003 (3 bytes)
  [units]                 : "GHz"
  [value length]          : 0x00000003 (3 bytes)
  [value]                 : "3.5"
```

Explanation:

- "cpu" is a container node.
Inside it:
- "temperature" = "75" "C"
- "frequency" = "3.5" "GHz"


This is already a tree-like structure, very similar to JSON:
```json
{
  "cpu": {
    "temperature": "75 C",
    "frequency": "3.5 GHz"
  }
}
```



### Example 3: Complex container with multiple subsystems

Now imagine a system health module that reports CPU and memory stats in one message.

```
Header:
[version]                 : 0x01 (1)
[payload size]            : 0x0000008B (139 bytes)

Payload (container "system"):
[node type]               : 0x00 (container)
[node name length]        : 0x00000006 (6 bytes)
[node name]               : "system"
[payload size]            : 0x0000007C (124 bytes)

  Child #1 (container "cpu"):      // identical to Example 2
    [node type]           : 0x00 (container)
    [node name length]    : 0x00000003 (3 bytes)
    [node name]           : "cpu"
    [payload size]        : 0x00000037 (55 bytes)
      ...two value nodes as above...
    // total size of "cpu" container = 67 bytes

  Child #2 (container "memory"):
    [node type]           : 0x00 (container)
    [node name length]    : 0x00000006 (6 bytes)
    [node name]           : "memory"
    [payload size]        : 0x0000002A (42 bytes)

      Grandchild #1 (value "total"):
      [node type]         : 0x01 (value)
      [node name length]  : 0x00000005 (5 bytes)
      [node name]         : "total"
      [units length]      : 0x00000002 (2 bytes)
      [units]             : "GB"
      [value length]      : 0x00000002 (2 bytes)
      [value]             : "16"

      Grandchild #2 (value "used"):
      [node type]         : 0x01 (value)
      [node name length]  : 0x00000004 (4 bytes)
      [node name]         : "used"
      [units length]      : 0x00000002 (2 bytes)
      [units]             : "GB"
      [value length]      : 0x00000001 (1 byte)
      [value]             : "7"
```

Explanation:

- Top-level container "system".

Contains:

- "cpu" container with temperature and frequency.

- "memory" container with total and used.

Conceptually, this is equivalent to:

```json
{
  "system": {
    "cpu": {
      "temperature": "75 C",
      "frequency": "3.5 GHz"
    },
    "memory": {
      "total": "16 GB",
      "used": "7 GB"
    }
  }
}
```
