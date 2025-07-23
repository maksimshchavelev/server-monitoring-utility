# Creating modules with static data
The last chapter described the creation of the RAM module. This module is constantly updating its data, but what if the data is static? After all, constantly creating a json tree ruins performance! Let's look at the solution in this chapter
We will first look at a possible **but not efficient approach** (although it is more efficient than creating a tree from scratch), and then at the **most efficient approach possible**

## Intuition
It would be nice if in the module constructor you could get all the data once and save the finished json tree

## Problem
The problem is that `make_root_node` returns a ***horrible huge*** type (see the dropdown). Plus, this type changes a lot when new nodes are added

<details>
  <summary>Return type</summary>
  
  ```
  std::unique_ptr<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricContainerNode
<std::unique_ptr<smu_server::internals::MetricValueNode, std::default_delete<smu_server::internals::MetricValueNode> >,
std::unique_ptr<smu_server::internals::MetricValueNode, std::default_delete<smu_server::internals::MetricValueNode> >,
std::unique_ptr<smu_server::internals::MetricValueNode, std::default_delete<smu_server::internals::MetricValueNode> > >,
std::default_delete<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > > > >,
std::unique_ptr<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > >,
std::default_delete<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > > > >,
std::unique_ptr<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > >,
std::default_delete<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > > > >,
std::unique_ptr<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > >,
std::default_delete<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >,
std::unique_ptr<smu_server::internals::MetricValueNode, std::default_delete<smu_server::internals::MetricValueNode> >,
std::unique_ptr<smu_server::internals::MetricValueNode, std::default_delete<smu_server::internals::MetricValueNode> >,
std::unique_ptr<smu_server::internals::MetricValueNode, std::default_delete<smu_server::internals::MetricValueNode> > > > > >,
std::default_delete<smu_server::internals::MetricContainerNode<std::unique_ptr<
smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > >,
std::default_delete<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > > > >,
std::unique_ptr<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > >,
std::default_delete<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > > > >,
std::unique_ptr<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > >,
std::default_delete<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >,
std::unique_ptr<smu_server::internals::MetricValueNode, std::default_delete<smu_server::internals::MetricValueNode> > > > >,
std::unique_ptr<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > >,
std::default_delete<smu_server::internals::MetricContainerNode<std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> >, std::unique_ptr<smu_server::internals::MetricValueNode,
std::default_delete<smu_server::internals::MetricValueNode> > > > > > > >
  ```

</details>


## Solution
Fortunately there is a solution. You just need to store the following:
```cpp
std::unique_ptr<IMetricNodeBase> m_root;
```

You can then assign `m_root` to your root node:
```cpp
m_root = make_root_node(/* some nodes here */);
```

Then you call `to_json` in `get_data` as usual:
```cpp
return m_root->to_json();
```

# A more productive approach
Why do we need to construct a json tree from scratch by calling `m_root->to_json()` when we can create it once in the constructor and just return it from `get_data`?
We should just store a field of type `Json::Value` in the module class and return it from `get_data`.
This method is far preferable!

# Summary
In this chapter, two approaches to creating a module that stores static data were discussed
