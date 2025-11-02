# Storing the entire JSON tree when creating modules
The [previous chapter (static data modules)](2_static_data_modules.md) described the optimization for modules with static data. In this chapter, you will learn what to do if you want to save the entire tree obtained via `make_root_node` (but you don't want to store the finished `Json::Value`)

> ⚠️ If you do this in your static data modules, it's a bad idea, as the `to_json` method causes recursive formation of `Json::Value`! It's better to use the approach from [the previous chapter](2_static_data_modules.md)!


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

# Summary
This chapter discussed an approach for storing the JSON tree obtained from `make_root_node`. [See next guide](4_mdtp.md)
