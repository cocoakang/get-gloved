---
layout: post
title: Rendering in Unity3d
tags: [frontpage, jekyll, blog]
image: '/images/posts/skeleton.png'
---

Finally, we can manipulate measured hand gesture data in Unity3d. First of all, we need to add skeleton to the mesh of glove using 3dsMax, like:
![SKELETON](http://cocoakang.cn/get-gloved/images/posts/add_skeleton.png)  
And a *skin* modifier should be add to the mesh and combine the mesh and skeleton together. It should be noted that we should create a root bone to apply transformation of the whole hand. At last, we export the skined hand as FBX file, and import this file to unity3d.  
  
### Pivot and target in IK
Since I only got one IMU for each finger, I cannot apply every motion to every joint of the skeleton. So [IK](https://en.wikipedia.org/wiki/Inverse_kinematics) is picked here. In IK system, a specific motion can be applied to sub objects which will drive its parent objects to transform and rotate. Unity3d has its built IK system. It can only, however, be applied to humanoid skeleton, while our hand skeleton is geneic type. On the other hand, there are a few scripts implements IK algorithm in Unity3d. I choosed this [one](https://assetstore.unity.com/packages/tools/animation/fast-ik-139972?locale=zh-CN) for free. :D  
  
You just need to attach this script to each end bone of fingers and choose what *target* it is aiming at. Wait, what doest *target* means? Don't worry, I will explain that.  
  
Back to our trouble raised in this block, when we can only measure the motion at the end of each finger, how can we calculate the whole finger's motion? Because our finger can only bend to one direction, we can transform the motion of end of the finger to how much it bends, and apply this bend to the bone.  
  
So what we need here is a pivot and a target object. We rotate the target around its pivot to drive whole the whole finger bends. Feel dizzy? Glance this picture:  
![PIVOT](http://cocoakang.cn/get-gloved/images/posts/pivot.png)
![TARGET](http://cocoakang.cn/get-gloved/images/posts/target.png)  

When a rotation is aplied to the target object, the finger will looks like:
![ROT](http://cocoakang.cn/get-gloved/images/posts/rot.png)  
  
### Solving local motion of a finger
The last thing we need to consider is how to solve the motion of the targer. Because the data got from IMUs at the end of fingers is global, it contains both local motion(bend finger) and global motion(move hand). Just subjecting global motion from the measure data works well. You ask where to find global motion? Don't forget we have another IMU attached at the opisthenar. The code seems like:
```C#
Quaternion anti_hand_rot = Quaternion.Inverse(hand_rot);
hand_rot_B_1 = anti_hand_rot * hand_rot_B_1;
```
The whole script can be found at /src/unity_part. I am not quite sure about the license applied to the glove model and FaskIK script, so only the script of controller is released.