import { useState, useEffect, useRef } from 'react';
import { Text, Box, FormLabel, Textarea, useToast, Flex, useColorModeValue, Button, Table, TableContainer, Td, Tbody, Thead, Tr, Th, TableCaption, Tfoot, VStack } from '@chakra-ui/react';
import { ArrowBackIcon } from '@chakra-ui/icons';
import * as Yup from "yup";
import { useNavigate } from "react-router";
import TextField from './Login/TextField';
import { Form, Formik } from "formik";
import Cookies from 'js-cookie';
import ConfirmDialog from './ConfirmDialog';

const EmailPage = () => {
  const [showCompose, setShowCompose] = useState(false);
  const [inbox, setInbox] = useState(null);
  const [viewingEmail, setViewingEmail] = useState("");
  const [composeState, setComposeState] = useState(0);
  const [body, setBody] = useState("");
  const [composeBody, setComposeBody] = useState("");
  const [inboxType, setInboxType] = useState(0);

  const composeFormRef = useRef(null);
  const toast = useToast();
  const initialRender = useRef(true);

  useOutsideAlerter(composeFormRef);

  function useOutsideAlerter(ref) {
    useEffect(() => {
      /**
       * Alert if clicked on outside of element
       */
      function handleClickOutside(event) {
        if (ref.current && !ref.current.contains(event.target)) {
          setShowCompose(false);
        }
      }
      // Bind the event listener
      document.addEventListener("mousedown", handleClickOutside);
      return () => {
        // Unbind the event listener on clean up
        document.removeEventListener("mousedown", handleClickOutside);
      };
    }, [ref]);
  }

  useEffect(() => {
    if (initialRender.current) {
      getInbox();
      initialRender.current = false;
    }
  }, []);

  useEffect(() => {
    getInbox();
  }, [inboxType]);

  const getInbox = () => {
    const body_info = {
      "sent_inbox": inboxType
    };
    const url = `http://${window.location.host}/api/webmail/inbox`;
    fetch(url, {
      method: 'POST',
      credentials: 'include',
      headers: {
        'Accept': 'application/json',
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(body_info),
    }).then(response => {
      if (!response.ok) {
        setInbox(null);
      } else {
        response.json().then((responseJson) => {
          console.log(responseJson);
          setInbox(responseJson);
        });
      }
    });
  };

  const deleteEmail = (email_id) => {
    const body_info = {
      "sent_inbox": inboxType,
      "email_id": email_id
    };
    const url = `http://${window.location.host}/api/webmail/delete`;
    fetch(url, {
      method: 'POST',
      credentials: 'include',
      headers: {
        'Accept': 'application/json',
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(body_info),
    }).then(response => {
      if (!response.ok) {
        toast({
          title: 'Error',
          description: "Failed to delete",
          status: 'error',
          duration: 3000,
          isClosable: true,
          position: 'top-center',
        });
      } else {
        toast({
          title: 'Message',
          description: "Deleted message",
          status: 'success',
          duration: 3000,
          isClosable: true,
          position: 'top-center',
        });
        getInbox();
        setViewingEmail("");
      }
    });
  };

  return (
    <Flex bg={useColorModeValue('gray.100', 'gray.900')} p={10} pl={0} pr={5} height={'100%'} overflow={'hidden'} position={'relative'}>
      {showCompose &&
        <VStack ref={composeFormRef} boxShadow='dark-lg' rounded='md' height={'70%'} width={'40%'} bg={'white'} position={'fixed'} bottom={0} right={7}>
          <Flex width='100%' bg='gray.200' p={2} pl={5} fontWeight='bold'>
            New Email
          </Flex>
          <Flex flexDir={'column'} width={'100%'} p={5} pb={5}>
            <Formik
              initialValues={{ subject: "", recipient: "" }}
              validationSchema={Yup.object({
                subject: Yup.string()
                  .required("Subject required!"),
                recipient: Yup.string()
                  .required("Recipient required!"),
              })}
              onSubmit={(values, actions) => {
                const vals = { ...values };
                const body_json = {
                  sender: Cookies.get('username') + "@localhost",
                  recipients: vals.recipient.split(","),
                  subject: vals.subject,
                  body: composeBody,
                };

                console.log(body_json);
                actions.resetForm();
                setComposeBody("");
                setViewingEmail("");
                setShowCompose(false);

                //change to ${window.location.host}
                const url = `http://${window.location.host}/api/webmail/send`;
                fetch(url, {
                  method: 'POST',
                  credentials: 'include',
                  headers: {
                    'Accept': 'application/json',
                    'Content-Type': 'application/json',
                  },
                  body: JSON.stringify(body_json),
                }).then(response => {
                  if (!response.ok) {
                    response.json().then((responseJson) => {
                      toast({
                        title: 'Error',
                        description: "Failed to send",
                        status: 'error',
                        duration: 3000,
                        isClosable: true,
                        position: 'top-center',
                      });
                      console.log(responseJson);
                    });
                  } else {
                    toast({
                      title: 'Message',
                      description: "Email successfully sent",
                      status: 'success',
                      duration: 3000,
                      isClosable: true,
                      position: 'top-center',
                    });
                    getInbox();
                  }

                });
              }}
              height='auto'
              width='100%'
            >
              <VStack as={Form} width='100%' m="auto" justify="center" spacing="1rem" alignItems='start'>
                <TextField mb={3} name="subject" placeholder="Enter subject" autoComplete="off" label="Subject" />
                <TextField mb={3} name="recipient" placeholder="Enter recipients" autoComplete="off" label="To" />
                <FormLabel textAlign='left'>Body</FormLabel>
                <Textarea mb={3} placeholder="Enter your message here" onChange={(e) => setComposeBody(e.target.value)} />
                <Button colorScheme="teal" type="submit">Send</Button>
              </VStack>
            </Formik >
          </Flex>
        </VStack>
      }
      <VStack width={'15%'} alignItems={'start'} pr={5}>
        <Button colorScheme='blue' mb={5} ml={5} width={'70%'} height={'50px'} fontSize={'16px'} onClick={() => setShowCompose(true)}>Compose</Button>
        <Flex fontSize={15} fontWeight={'bold'}
          bg={inboxType == 0 ? 'blackAlpha.300' : ''} color={inboxType == 0 ? 'white' : 'black'} width={'100%'} p={8} pt={2} pb={2}
          roundedRight={'2xl'} onClick={() => setInboxType(0)}>
          Inbox
        </Flex>
        <Flex fontSize={15} fontWeight={'bold'}
          bg={inboxType == 1 ? 'blackAlpha.300' : ''} color={inboxType == 1 ? 'white' : 'black'}
          width={'100%'} p={8} pt={2} pb={2} roundedRight={'2xl'} onClick={() => setInboxType(1)}>
          Sent
        </Flex>
      </VStack>
      <Flex bg={'white'} width={'85%'} rounded={'lg'} height={'100%'} overflow={'scroll'}>
        {inbox && inbox["emails"] ?
          (viewingEmail) ?
            <VStack width={'100%'} alignItems={'start'}>
              <Flex onClick={() => { setViewingEmail(null); setComposeState(0); }} bg={'blackAlpha.200'} justifyContent={'start'} width={'100%'} p={5} fontWeight={'bold'}>
                <ArrowBackIcon alignSelf='center' mr={5} />
                Back
              </Flex>
              <Flex borderBottom='2px' flexDir={'column'} borderBottomColor={'blackAlpha.100'} width={'100%'} p={5} pb={8}>
                {console.log(viewingEmail)}
                <Flex>
                  <Text flex='1' fontSize={30} fontWeight={'bold'}>{viewingEmail["threads"][0]["subject"]}</Text>
                  <Button colorScheme='blue' width={'90px'} height={'50px'} fontSize={'16px'} onClick={() => setComposeState(1)} mr={5}>Reply</Button>
                  <Button colorScheme='blue' width={'90px'} height={'50px'} fontSize={'16px'} onClick={() => setComposeState(2)} mr={5}>Forward</Button>
                  <ConfirmDialog label="Delete" dialogText="Delete Email" onConfirm={() => deleteEmail(viewingEmail["email_id"])} />
                </Flex>
                <Text fontSize={20} color={'blackAlpha.700'}>From: {viewingEmail["threads"][0]["sender"]}</Text>
                <Text fontSize={20} color={'blackAlpha.700'}>To: {viewingEmail["threads"][0]["recipients"].map((recipient, i) => {
                  if (i == 0) {
                    return recipient;
                  } else {
                    return ", " + recipient;
                  }
                })}</Text>
                <Text fontSize={20} color={'blackAlpha.700'}>Date: {viewingEmail["threads"][0]["date"]}</Text>
                <Text fontSize={20} pt={8}>{viewingEmail["threads"][0]["body"]}</Text>
                {viewingEmail["threads"].map((email, i) => {
                  console.log(inbox);
                  if (i != 0) {
                    return (
                      <Flex borderLeft='2px' flexDir={'column'} borderLeftColor={'green.300'} width={'auto'} p={5} pb={2} mt={2} ml={(i - 1) * 5} pl={2}>
                        <Text fontSize={20} color={'blackAlpha.700'} fontWeight={'bold'}>{viewingEmail["threads"][i - 1]["forwarded"] == 0 ? "Reply to:" : "Forward message:"}</Text>
                        <Text fontSize={20} color={'blackAlpha.700'}>Subject: {email["subject"]}</Text>
                        <Text fontSize={20} color={'blackAlpha.700'}>From: {email["sender"]}</Text>
                        <Text fontSize={20} color={'blackAlpha.700'}>To: {email["recipients"].map((recipient, i) => {
                          if (i == 0) {
                            return recipient;
                          } else {
                            return ", " + recipient;
                          }
                        })}</Text>
                        <Text fontSize={20} color={'blackAlpha.700'}>Date: {email["date"]}</Text>
                        <Text fontSize={20} pt={5}>{email["body"]}</Text>
                      </Flex>);
                  }
                })}
              </Flex>
              {composeState > 0 && <Flex flexDir={'column'} width={'100%'} p={5} pb={5}>
                <Formik
                  initialValues={{ subject: "", recipient: "" }}
                  validationSchema={Yup.object({
                    subject: Yup.string()
                      .required("Subject required!"),
                    recipient: Yup.string()
                      .required("Recipient required!"),
                  })}
                  onSubmit={(values, actions) => {
                    const vals = { ...values };
                    const body_json = {
                      sender: Cookies.get('username') + "@localhost",
                      recipients: vals.recipient.split(","),
                      subject: vals.subject,
                      body: body,
                    };

                    if (composeState == 1) {
                      body_json["reply"] = viewingEmail["email_id"];
                    } else {
                      body_json["forward"] = viewingEmail["email_id"];
                    }

                    console.log(body_json);
                    actions.resetForm();
                    setBody("");
                    setViewingEmail("");
                    setComposeState(0);

                    const url = `http://${window.location.host}/api/webmail/` + (composeState == 1 ? "reply" : "forward");

                    //change to ${window.location.host}
                    fetch(url, {
                      method: 'POST',
                      credentials: 'include',
                      headers: {
                        'Accept': 'application/json',
                        'Content-Type': 'application/json',
                      },
                      body: JSON.stringify(body_json),
                    }).then(response => {
                      if (!response.ok) {
                        response.json().then((responseJson) => {
                          toast({
                            title: 'Error',
                            description: "Failed to " + (composeState == 1 ? "reply" : "forward"),
                            status: 'error',
                            duration: 3000,
                            isClosable: true,
                            position: 'top-center',
                          });
                          console.log(responseJson);
                        });
                      } else {
                        toast({
                          title: 'Message',
                          description: "Email successfully " + (composeState == 1 ? "replied to" : "forwarded"),
                          status: 'success',
                          duration: 3000,
                          isClosable: true,
                          position: 'top-center',
                        });
                        getInbox();
                      }
                    });
                  }}
                  height='auto'
                  width='100%'
                >
                  <VStack as={Form} width='100%' m="auto" justify="center" spacing="1rem">
                    <Box p={8} width='100%' maxWidth="5000px" borderWidth={1} borderRadius={8} boxShadow="lg">
                      <TextField mb={3} name="subject" placeholder="Enter subject" autoComplete="off" label="Subject" />
                      <TextField mb={3} name="recipient" placeholder="Enter recipients" autoComplete="off" label={composeState == 1 ? "Reply to" : "Forward to"} />
                      <FormLabel>Body</FormLabel>
                      <Textarea mb={3} placeholder="Enter your message here" onChange={(e) => setBody(e.target.value)} />
                      <Button colorScheme="teal" type="submit">{composeState == 1 ? "Reply" : "Forward"}</Button>
                    </Box>
                  </VStack>
                </Formik >
              </Flex>}

            </VStack>
            :
            <Flex width={'100%'} flexDir={'column'}>
              <Text p={5} fontSize={16} fontWeight={'bold'}>{inboxType == 0 ? 'Inbox' : 'Sent'} ({inbox["emails"].length})</Text>
              <TableContainer width={'100%'} p={5} pt={0} overflowY={'scoll'}>
                <Table variant='simple'>
                  <Thead>
                    <Tr>
                      <Th>From</Th>
                      <Th>Subject</Th>
                      <Th>Body</Th>
                      <Th>Date</Th>
                    </Tr>
                  </Thead>
                  <Tbody>
                    {
                      inbox["emails"].slice(0).reverse().map((data) => {
                        return (
                          <Tr onClick={() => setViewingEmail(data)}>
                            <Td>{data["threads"][0]["sender"]}</Td>
                            <Td>{data["threads"][0]["subject"]}</Td>
                            <Td>{data["threads"][0]["body"]}</Td>
                            <Td>{data["threads"][0]["date"]}</Td>
                          </Tr>
                        );
                      })
                    }
                  </Tbody>
                  <Tfoot>
                  </Tfoot>
                </Table>
              </TableContainer>
            </Flex>
          :
          <Text textAlign='center' width='100%' pt={8} fontWeight={'bold'}>Empty inbox</Text>
        }
      </Flex>
    </Flex>
  );
};

export default EmailPage;