using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;

public class glove : MonoBehaviour {
    public string IP = "169.254.210.41";//"192.168.0.3";
    public int port = 5791;
    public bool simulating = false;
    public GameObject pivot_B_5;
    public GameObject pivot_B_4;
    public GameObject pivot_B_2;
    public GameObject pivot_B_3;
    public GameObject pivot_B_1;
    public float test_angle4 = 0.0f;
    public float test_angle2 = 0.0f;
    public float test_angle3 = 0.0f;
    public float test_angle1 = 0.0f;
    public float test_angle = 0.0f;
    IPEndPoint remoteEndPoint;
    UdpClient client;
    Vector3 word_rot_axis;

    byte[] glove_state_req;

    GameObject B_5_target;
    GameObject B_4_target;
    GameObject B_2_target;
    GameObject B_3_target;
    GameObject B_1_target;
    GameObject whole_hand;
    Animator anim;
    Quaternion hand_rot_B_4;
    Quaternion hand_rot_B_2;
    Quaternion hand_rot_B_3;
    Quaternion hand_rot_B_1;
    Quaternion hand_rot;

    private Vector3 originalPosition_B_5;
    private Quaternion originalRotation_B_5;
    private Vector3 originalPosition_B_4;
    private Quaternion originalRotation_B_4;
    private Vector3 originalPosition_B_2;
    private Quaternion originalRotation_B_2;
    private Vector3 originalPosition_B_3;
    private Quaternion originalRotation_B_3;
    private Vector3 originalPosition_B_1;
    private Quaternion originalRotation_B_1;
    private Vector3 originalPosition;
    private Quaternion originalRotation;

    bool first_update = true;

    void Awake()
    {
        hand_rot_B_1 = Quaternion.identity;
        hand_rot_B_2 = Quaternion.identity;
        hand_rot_B_3 = Quaternion.identity;
        hand_rot_B_4 = Quaternion.identity;
        if (simulating)
        {
            Debug.Log("hand rot using simulating data");
            word_rot_axis.Set(1.0f, 0.0f, 0.0f);
            hand_rot = Quaternion.AngleAxis(test_angle, word_rot_axis);
        }
        else
        {
            hand_rot = Quaternion.identity;
        }


        B_1_target = GameObject.Find("/infinity_gauntlet/root/B_1_target");
        B_2_target = GameObject.Find("/infinity_gauntlet/root/B_2_target");
        B_3_target = GameObject.Find("/infinity_gauntlet/root/B_3_target");
        B_4_target = GameObject.Find("/infinity_gauntlet/root/B_4_target");
        B_5_target = GameObject.Find("/infinity_gauntlet/root/B_5_target");
        whole_hand = GameObject.Find("/infinity_gauntlet");
        {
            if (B_2_target != null)
            {
                Debug.Log("Found B_2_target!");
            }
            else
            {
                Debug.Log("Cannot find B_2_target.");
            }
            if (B_3_target != null)
            {
                Debug.Log("Found B_3_target!");
            }
            else
            {
                Debug.Log("Cannot find B_3_target.");
            }
            if (B_4_target != null)
            {
                Debug.Log("Found B_4_target!");
            }
            else
            {
                Debug.Log("Cannot find B_4_target.");
            }
            if (B_1_target != null)
            {
                Debug.Log("Found B_1_target!");
            }
            else
            {
                Debug.Log("Cannot find B_1_target.");
            }
            if (B_5_target != null)
            {
                Debug.Log("Found B_5_target!");
            }
            else
            {
                Debug.Log("Cannot find B_5_target.");
            }
            if (whole_hand != null)
            {
                Debug.Log("Found whole_hand!");
            }
            else
            {
                Debug.Log("Cannot find whole_hand.");
            }
        }
        

        
    }

    // Use this for initialization
    void Start () {
        this.originalPosition_B_1 = B_1_target.transform.localPosition;
        this.originalRotation_B_1 = B_1_target.transform.localRotation;
        this.originalPosition_B_2 = B_2_target.transform.localPosition;
        this.originalRotation_B_2 = B_2_target.transform.localRotation;
        //Debug.Log("origin position B_2:" + this.originalPosition_B_2);
        //Debug.Log("origin Rotation B_2:" + this.originalRotation_B_2);
        this.originalPosition_B_3 = B_3_target.transform.localPosition;
        this.originalRotation_B_3 = B_3_target.transform.localRotation;
        this.originalPosition_B_4 = B_4_target.transform.localPosition;
        this.originalRotation_B_4 = B_4_target.transform.localRotation;
        this.originalPosition_B_5 = B_5_target.transform.localPosition;
        this.originalRotation_B_5 = B_5_target.transform.localRotation;
        this.originalPosition = whole_hand.transform.position;
        this.originalRotation = whole_hand.transform.rotation;

        remoteEndPoint = new IPEndPoint(IPAddress.Parse(IP), port);
        client = new UdpClient();
        client.Client.ReceiveTimeout = 3000;//msec

        byte PT_GRAB_GLOVE_STATE = 0x3;

        glove_state_req = Enumerable.Repeat(PT_GRAB_GLOVE_STATE,(byte)0x1).ToArray();

        anim = this.GetComponent<Animator>();
    }
    
    // Update is called once per frame
    void Update () {
        try
        {
            this.resetTransform();

            if (!simulating)
            {
                client.Send(glove_state_req, glove_state_req.Length, remoteEndPoint);
                byte[] byteArray = client.Receive(ref remoteEndPoint);
                var floatArray2 = new float[byteArray.Length / 4];
                Buffer.BlockCopy(byteArray, 0, floatArray2, 0, byteArray.Length);

                hand_rot.Set(floatArray2[13], -floatArray2[15], floatArray2[14], floatArray2[12]);
                hand_rot_B_1.Set(floatArray2[17], -floatArray2[19], floatArray2[18], floatArray2[16]);
                hand_rot_B_2.Set(floatArray2[1], -floatArray2[3], floatArray2[2], floatArray2[0]);
                hand_rot_B_3.Set(floatArray2[5], -floatArray2[7], floatArray2[6], floatArray2[4]);
                hand_rot_B_4.Set(floatArray2[9], -floatArray2[11], floatArray2[10], floatArray2[8]);
            }
            else
            {
                hand_rot = Quaternion.AngleAxis(test_angle, word_rot_axis);
                hand_rot_B_1 = hand_rot * hand_rot_B_1;
                hand_rot_B_2 = Quaternion.identity;
                hand_rot_B_2 = hand_rot * hand_rot_B_2;
                //word_rot_axis = new Vector3(0.0f, 1.0f, 0.0f);
                hand_rot_B_2 = Quaternion.AngleAxis(test_angle2, word_rot_axis)*hand_rot_B_2;
                //word_rot_axis = new Vector3(1.0f, 0.0f, 0.0f);
                hand_rot_B_3 = hand_rot * hand_rot_B_3;
                hand_rot_B_4 = hand_rot * hand_rot_B_4;
            }
            
            Quaternion anti_hand_rot = Quaternion.Inverse(hand_rot);

            hand_rot_B_1 = anti_hand_rot * hand_rot_B_1;
            hand_rot_B_2 = anti_hand_rot * hand_rot_B_2;
            hand_rot_B_3 = anti_hand_rot * hand_rot_B_3;
            hand_rot_B_4 = anti_hand_rot * hand_rot_B_4;

            float tmp_angle;
            Vector3 tmp_axis;
            var rot = whole_hand.transform.rotation;
            rot.Set(hand_rot.x, hand_rot.y, hand_rot.z, hand_rot.w);
            
            hand_rot_B_1.ToAngleAxis(out tmp_angle, out tmp_axis);
            B_1_target.transform.RotateAround(pivot_B_1.transform.position, tmp_axis, tmp_angle);

            if (first_update)
            {
                //first_update = false;
                hand_rot_B_2.ToAngleAxis(out tmp_angle, out tmp_axis);

                //Debug.Log("x:" + hand_rot_B_2.eulerAngles.x + " y:" + hand_rot_B_2.eulerAngles.y + " z:" + hand_rot_B_2.eulerAngles.z);
                //Debug.Log("local rotation:" + B_2_target.transform.localRotation.eulerAngles);
                //Debug.Log("rotation:" + B_2_target.transform.rotation.eulerAngles);
                //Debug.Log("rotation axis:" + tmp_angle + " axis:"+ tmp_axis);
                //Debug.Log("origin position:" + B_2_target.transform.position);
                //Debug.Log("pivot position:" + pivot_B_2.transform.position);
                //var tmp_pos = B_2_target.transform.position;
                //tmp_pos = tmp_pos - pivot_B_2.transform.position;
                //tmp_pos = hand_rot_B_2 * tmp_pos + pivot_B_2.transform.position;
                //B_2_target.transform.position = tmp_pos;
                //Vector3 rot_axsi = new Vector3(-1.0f, 0.0f, 0.0f);
                B_2_target.transform.RotateAround(pivot_B_2.transform.position, tmp_axis, tmp_angle);
                //Debug.Log("result position:" + B_2_target.transform.position);
                //Debug.Log("local rotation:" + B_2_target.transform.localRotation.eulerAngles);
                //Debug.Log("rotation:" + B_2_target.transform.rotation.eulerAngles);

            }

            hand_rot_B_3.ToAngleAxis(out tmp_angle, out tmp_axis);
            B_3_target.transform.RotateAround(pivot_B_3.transform.position, tmp_axis, tmp_angle);

            hand_rot_B_4.ToAngleAxis(out tmp_angle, out tmp_axis);
            B_4_target.transform.RotateAround(pivot_B_4.transform.position, tmp_axis, tmp_angle);
            B_5_target.transform.RotateAround(pivot_B_5.transform.position, tmp_axis, tmp_angle);

            whole_hand.transform.rotation = rot;

        }
        catch (Exception err)
        {
            Debug.LogError(err.ToString());
        }

    }

    // call this function to reset this script's object's position and rotation
    public void resetTransform()
    {
        B_4_target.transform.localPosition = this.originalPosition_B_4;
        B_4_target.transform.localRotation = this.originalRotation_B_4;

        if (first_update)
        {
            B_2_target.transform.localPosition = this.originalPosition_B_2;
            B_2_target.transform.localRotation = this.originalRotation_B_2;
            //Debug.Log("rest position B_2:" + B_2_target.transform.position);
            //Debug.Log("rest Rotation B_2:" + B_2_target.transform.rotation);
        }


        B_3_target.transform.localPosition = this.originalPosition_B_3;
        B_3_target.transform.localRotation = this.originalRotation_B_3;

        B_1_target.transform.localPosition = this.originalPosition_B_1;
        B_1_target.transform.localRotation = this.originalRotation_B_1;

        B_5_target.transform.localPosition = this.originalPosition_B_5;
        B_5_target.transform.localRotation = this.originalRotation_B_5;

        whole_hand.transform.position = this.originalPosition;
        whole_hand.transform.rotation = this.originalRotation;
    }
}