import { useState, useEffect, useRef } from 'react'
import { 
	Text, 
	Menu, 
	MenuButton, 
	MenuList, 
	MenuItem, 
	Box, 
	FormLabel, 
	useToast, 
	Flex, 
	useColorModeValue, 
	Button, 
	Table, 
	TableContainer, 
	Td, 
	Tbody, 
	Thead, 
	Tr, 
	Th, 
	Tfoot, 
	VStack,
	Image,
	Spinner} from '@chakra-ui/react';
import { AddIcon, StarIcon, HamburgerIcon } from '@chakra-ui/icons'
import * as Yup from "yup";
import { useNavigate } from "react-router";
import TextField from './Login/TextField';
import { Form, Formik } from "formik";
import Cookies from 'js-cookie'
import StorageActionModal from './StorageActionModal';

const joinUpTo = (array, max, driveType) => {
	let joined = "";
	for (let i = 0; i <= max; i++) {
		if (i == 0) {
			joined += "/";
			continue;
		} else if (i == 2 && driveType != 0) {
			joined += "/";
		}

		joined += array[i];
	}

	return joined;
}

const StoragePage = () => {
	const inputRef = useRef(null);
	const [path, setPath] = useState('/');
	const [drive, setDrive] = useState(null);
	const [showActionModal, setShowActionModal] = useState(false);
	const [actionType, setActionType] = useState(0);
	const [driveType, setDriveType] = useState(0);
	const [id, setId] = useState("");
	const [isUploading, setIsUploading] = useState(false);
 
	const linkRef = useRef(null);
	const toast = useToast();

	const onUploadClick = () => {
		if (inputRef?.current) {
			inputRef.current.value = "";
            inputRef.current.click();
		}
	}

	const fileUploadChanged = async (e) => {
		let file = e.target.files[0];
		const formData = new FormData();
  		formData.append('file', file);
		console.log(formData);
		setIsUploading(true);
		const url = `http://${window.location.host}/api/filesystem/upload` + path;
		await fetch(url, {
			method: 'POST',
			credentials: 'include',
			headers: {
				'Accept': 'application/json',
				'Content-Type': 'multipart/form-data',
			},
			body: formData
		}).then(response => {
			if (!response.ok) {
				toast({
					title: 'Error',
					description: "Failed to upload file", 
					status: 'error',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				  });
			} else {
				toast({
					title: 'Success',
					description: "File uploaded",
					status: 'success',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				});
				navigate();
			}
		})
		setIsUploading(false);
	}

	useEffect(() => {
		navigate();
	}, [path])

	const navigate = () => {
		const bodyJson = {
			path: path
		}
		const url = `http://${window.location.host}/api/filesystem/navigate`;
		fetch(url, {
			method: 'POST',
			credentials: 'include',
			headers: {
				'Accept': 'application/json',
				'Content-Type': 'application/json',
			},
			body: JSON.stringify(bodyJson)
		}).then(response => {
			if (!response.ok) {
			} else {
				response.json().then((responseJson) => {
					setDrive(responseJson);
				})
			}
		})
	}

	const onStar = (e, id, starred) => {
		e.stopPropagation();
		const bodyJson = {
			id: id,
			star: !starred
		}
		const url = `http://${window.location.host}/api/filesystem/star`;
		fetch(url, {
			method: 'POST',
			credentials: 'include',
			headers: {
				'Accept': 'application/json',
				'Content-Type': 'application/json',
			},
			body: JSON.stringify(bodyJson)
		}).then(response => {
			if (!response.ok) {
				toast({
					title: 'Error',
					description: "Failed to" + (!starred ? "delete from " : "add to ") + "starred", 
					status: 'error',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				  });
			} else {
				toast({
					title: 'Success',
					description: (starred ? "Deleted from " : "Added to ") + "starred",
					status: 'success',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				});
				navigate();
			}
		})
	}

	const onDelete = (e, id) => {
		e.stopPropagation();
		const bodyJson = {
			path: path,
			id: id
		}
		const url = `http://${window.location.host}/api/filesystem/delete`;
		fetch(url, {
			method: 'POST',
			credentials: 'include',
			headers: {
				'Accept': 'application/json',
				'Content-Type': 'application/json',
			},
			body: JSON.stringify(bodyJson)
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
					title: 'Success',
					description: "Deleted",
					status: 'success',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				});
				navigate();
			}
		})
	}

	const onRename = (e, id) => {
		e.stopPropagation();
		setShowActionModal(true);
		setActionType(1);
		setId(id);
	}

	const onCreateNewFolder = () => {
		setShowActionModal(true);
		setActionType(0);
	}

	const onMove = (e, id) => {
		e.stopPropagation();
		setShowActionModal(true);
		setActionType(2);
		setId(id);
	}

	const onDownload = (e, id, name) => {
		e.stopPropagation();
		const bodyJson = {
			id: id
		}
		if (linkRef && linkRef.current) {
			const url = `http://${window.location.host}/api/filesystem/download`;
			fetch(url, {
				method: 'POST',
				credentials: 'include',
				headers: {
					'Content-Type': 'application/json',
				},
				body: JSON.stringify(bodyJson)
			}).then((response) => {
				return response.blob();
			}).then(blob => {
				const href = window.URL.createObjectURL(blob);
				const a = linkRef.current;
				a.download = name;
				a.href = href;
				a.click();
				a.href = '';
			})
		}
		
	}

	const onActionClicked = (e) => {
		e.stopPropagation();
	}

	return (
		
		<Flex bg={useColorModeValue('gray.100', 'gray.900')} p={10} pl={0} pr={5} height={'100%'} overflow={'hidden'} position={'relative'}>
			{isUploading && <VStack boxShadow='dark-lg' rounded='md' height={'30%'} width={'30%'} bg={'white'} position={'fixed'} bottom={0} right={7}>
			<Flex width='100%' bg='gray.200' p={2} pl={5} fontWeight='bold'>
				Uploading
			</Flex>
          	<Flex flexDir={'column'} width={'100%'} p={5} pb={5} justifyContent={'center'} alignItems={'center'}>
			  <Spinner
				thickness='4px'
				speed='0.65s'
				emptyColor='gray.200'
				color='blue.500'
				size='xl'
				/>
			</Flex>
        	</VStack>}	
			<VStack width={'15%'} alignItems={'start'} pr={5}>
				<Menu>
				{({ isOpen }) => (
					<>
					<MenuButton colorScheme='blue' fontSize={'16px'} mb={5} ml={5} width={'70%'} height={'50px'} isActive={isOpen} as={Button} leftIcon={<AddIcon boxSize={3} />}>
						New
					</MenuButton>
					<MenuList>
						<MenuItem htmlFor="filePicker" as={Button}  onClick={onUploadClick} fontSize={15} fontWeight={'medium'} justifyContent={'start'}>New File</MenuItem>
						<MenuItem as={Button} onClick={onCreateNewFolder} fontSize={15} fontWeight={'medium'} justifyContent={'start'}>New Folder</MenuItem>
					</MenuList>
					</>
				)}
				</Menu>
					{/* <Button htmlFor="filePicker" colorScheme='blue' mb={5} ml={5} width={'70%'} height={'50px'} fontSize={'16px'} onClick={onUploadClick}>New</Button> */}
				{/* <Button htmlFor="filePicker" colorScheme='blue' mb={5} ml={5} width={'70%'} height={'50px'} fontSize={'16px'} onClick={download}>Download</Button> */}
				<input ref={inputRef} id="filePicker" style={{visibility:"hidden", position:'fixed'}} type={"file"} onChange={(e) => fileUploadChanged(e)}/>
				<a ref={linkRef} style={{visibility:"hidden", position:'fixed'}}/>
				<Flex fontSize={15} fontWeight={'bold'}
					bg={driveType == 0 ? 'blackAlpha.300' : ''} color={driveType == 0 ? 'white' : 'black'}  width={'100%'} p={8} pt={2} pb={2} 
					roundedRight={'2xl'} onClick={() => {setDriveType(0); setPath("/");}}>
					My Drive
				</Flex>
				<Flex fontSize={15} fontWeight={'bold'}
					bg={driveType == 1 ? 'blackAlpha.300' : ''} color={driveType == 1 ? 'white' : 'black'}  width={'100%'} p={8} pt={2} pb={2} 
					roundedRight={'2xl'} onClick={() => {setDriveType(1); setPath("/Recents");}}>
					Recents
				</Flex>
				<Flex fontSize={15} fontWeight={'bold'}
					bg={driveType == 2 ? 'blackAlpha.300' : ''} color={driveType == 2 ? 'white' : 'black'}  width={'100%'} p={8} pt={2} pb={2} 
					roundedRight={'2xl'} onClick={() => {setDriveType(2); setPath("/Trash");}}>
					Trash
				</Flex>
				{showActionModal && <StorageActionModal showStorageModal={showActionModal} setShowStorageModal={setShowActionModal} path={path} getNavigate={navigate} id={id} actionType={actionType}/>}
			</VStack>
			<Flex bg={'white'} width={'85%'} rounded={'lg'} height={'100%'} overflow={'scroll'}>
				<Flex width={'100%'} flexDir={'column'}>
				<Flex mb={5}>
					<Text pt={5} pl={5} fontSize={16} fontWeight={'bold'}>{driveType == 0 ? 
					"My Drive" : driveType == 1 ? "Recents" : "Trash"} (
					</Text>
					{path.split("/").map((subdir, i) => {
						const splitedPath = path.split("/");
						if (i > 0 && subdir == "") {
							return;
						}
						if (subdir == "") {
							if (driveType == 0) {
								return (
								<>
									<Text pt={5} fontSize={16} fontWeight={'bold'} onClick={() => setPath("/")} color={'blue'}>root</Text>
									{splitedPath.length > 1 && splitedPath[1] != "" && <Text pt={5} ml={1} mr={1} fontSize={16} fontWeight={'bold'}>/</Text>}
								</>
								)
							}
						} else {
							if (i == splitedPath.length - 1) {
								return <Text pt={5} fontSize={16} fontWeight={'bold'} onClick={() => setPath(joinUpTo(splitedPath, i, driveType))} color={'blue'}>{subdir}</Text>
							}
							return <>
								<Text pt={5} fontSize={16} fontWeight={'bold'} onClick={() => setPath(joinUpTo(splitedPath, i, driveType))} color={'blue'}>{subdir}</Text>
								<Text pt={5} ml={1} mr={1} fontSize={16} fontWeight={'bold'}>/</Text>
							</>
						}
					})}
					<Text pt={5} fontSize={16} fontWeight={'bold'}>)</Text>
				</Flex>
				{drive && drive.length != 0 && (path != "/" || drive.length > 2) ? 
				<TableContainer width={'100%'} p={5} pt={0} overflowY={'scoll'}>
					<Table variant='simple'>
						<Thead>
						<Tr>
							<Th width={'40%'}>Name</Th>
							<Th width={'13.75%'}>Owner</Th>
							<Th width={'13.75%'}>Last Modified</Th>
							<Th width={'13.75%'}>Size</Th>
							<Th width={'13.75%'}>Starred</Th>
							<Th width={'5%'}></Th>
						</Tr>
						</Thead>
						<Tbody>
						{drive.map((data) => {
							const isFolder = data["metadata"]["content_type"] ? false : true;
							if (data["id"] != "trash" && data["id"] != "recents") {
								return (
									<Tr onClick={() => {
										if (isFolder) {
											if (path == "/") {
												setPath(path + data["name"]);
											} else {
												setPath(path + "/" + data["name"]);
											}
										}
									}}>
										<Td display={'flex'} width={'40%'}><Image src={isFolder ? "https://i.imgur.com/ixheIqD.png" : "https://i.imgur.com/tK8Su2p.png"} height={5} mr={2}/>{data["name"]}</Td>
										<Td width={'13.75%'}>{data["metadata"]["owner"]}</Td>
										<Td width={'13.75%'}>{new Date(data["metadata"]["date created"] * 1000).toString().split(" ").slice(0, 5).join(" ")}</Td>
										<Td width={'13.75%'}>{data["metadata"]["size"] && data["metadata"]["size"]}</Td>
										<Td width={'13.75%'}>{data["metadata"]["starred"] ? <StarIcon color={'yellow.300'}></StarIcon> : <StarIcon color={'gray'}></StarIcon>}</Td>
										<Menu>
											{({ isOpen }) => (
												<>
												<MenuButton as={Td} width={'5%'} onClick={(e) => onActionClicked(e)}>
													<HamburgerIcon />
												</MenuButton>
												<MenuList>
													{!isFolder && <MenuItem as={Button} onClick={(e) => onDownload(e, data["id"], data["name"])} fontSize={15} fontWeight={'medium'} justifyContent={'start'}>Download</MenuItem>}
													<MenuItem as={Button} onClick={(e) => onMove(e, data["id"])} fontSize={15} fontWeight={'medium'} justifyContent={'start'}>Move</MenuItem>
													<MenuItem as={Button} onClick={(e) => onRename(e, data["id"])} fontSize={15} fontWeight={'medium'} justifyContent={'start'}>Rename</MenuItem>
													<MenuItem as={Button} onClick={(e) => onDelete(e, data["id"])} fontSize={15} fontWeight={'medium'} justifyContent={'start'}>Delete</MenuItem>
													<MenuItem as={Button} onClick={(e) => onStar(e, data["id"], data["metadata"]["starred"])} fontSize={15} fontWeight={'medium'} justifyContent={'start'}>{data["metadata"]["starred"] ? "Delete from " : "Add to "} starred</MenuItem>
												</MenuList>
												</>
											)}
										</Menu>
									</Tr>
								)
							}
						})}
						</Tbody>
						<Tfoot>
						</Tfoot>
					</Table>
				</TableContainer>
				: <Text textAlign='center' width='100%' pt={8} fontWeight={'bold'}>Empty directory</Text>
				}
				</Flex>
			</Flex>
		</Flex>
	)
}

export default StoragePage;